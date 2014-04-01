#include <stdlib.h>
#include <string.h>
#include "GL/glut.h"


#define TIMER 33

GLUquadric *myQuad;


static float xrot;

#define TMMODE_NONE 1
#define TMMODE_SINGLE 2
#define TMMODE_TWOPASS 3
#ifdef GL_HP_texture_lighting
#define TMMODE_HPEXT 4
#endif /* GL_HP_texture_lighting */
static int tmMode;
#define USE_ALPHA_TEST 10
static int alphaTest = 0;

static GLuint textures[2];


static void display( void )
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLoadIdentity ();
    gluLookAt (0., 0., 5.,
               0., 0., 0.,
               0., 1., 0.);

    glRotatef (xrot, 1., 0., 0.);
    glTranslatef (0., 0., -1.);

    /* Only pass or first of two passes */
    gluCylinder (myQuad, .5, .5, 2., 24, 8);
    
    if (tmMode == TMMODE_TWOPASS) {
        GLfloat white[4] = {1., 1., 1., 1.};

        /* second pass here */
        glPushAttrib (GL_ALL_ATTRIB_BITS);
        glColor3f (0., 0., 0.);
        glDepthFunc (GL_LEQUAL);
        glEnable (GL_BLEND);
        glLightfv (GL_LIGHT1, GL_SPECULAR, white);
        glBindTexture (GL_TEXTURE_2D, textures[1]);
        
        gluCylinder (myQuad, .5, .5, 2., 24, 8);
        
        glPopAttrib ();

        /* Bug! Should have popped... */
        glBindTexture (GL_TEXTURE_2D, textures[0]);
    }

    glFlush();
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport (0, 0, w, h);       
    glMatrixMode (GL_PROJECTION);  
    glLoadIdentity ();
    gluPerspective (50., (float)w/(float)h, 1., 20.);

    /* Leave us in modelview mode for our display routine */
    glMatrixMode(GL_MODELVIEW);
}

static void cbMainMenu (int value)
{
    GLfloat white[4] = {1., 1., 1., 1.};
    GLfloat black[4] = {0., 0., 0., 0.};

    
    if (value == 99) {
        exit (0);
    }

    if (value < USE_ALPHA_TEST)
        tmMode = value;

        glEnable (GL_TEXTURE_2D);
        glLightfv (GL_LIGHT1, GL_SPECULAR, white);
#ifdef GL_HP_texture_lighting
        glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_LIGHTING_MODE_HP, GL_TEXTURE_POST_SPECULAR_HP);
#endif /* GL_HP_texture_lighting */
}

static void timer (int value)
{
    xrot += 1.f;
    if (xrot > 360.f) xrot -= 360.f;

    glutPostRedisplay ();
    glutTimerFunc (TIMER, timer, 0);
}

static void initTextureMap (int w, int h, unsigned char *tm)
{
    int i, j, ci=w>>2, cj=h>>2;

    for (i=0; i<w; i++) {
        for (j=0; j<h; j++) {
            tm[0] = tm[1] = tm[2] = tm[3] = 0;
            if ((i&ci)^(j&cj)) {
                /* red has full alpha, black has zero alpha */
                tm[0] = tm[3] = 255;
            }
            tm += 4;
        }
    }
}

static void init ()
{
    int mainMenu;
    GLfloat pos[4] = {3., 5., 2., 1.};
    GLfloat white[4] = {1., 1., 1., 1.};
    
    xrot = 0.;

    glClearColor (.3, .3, .3, 0.);
    glColor4f (.5, .5, .5, 1.); /* cylinder is grey by default */
    glDisable (GL_DITHER);
    glEnable (GL_DEPTH_TEST);

    /* Set up alpha test to toss 0 alpha */
    glAlphaFunc (GL_NOTEQUAL, 0);

    /* blending function used in TMMODE_TWOPASS */
    glBlendFunc (GL_ONE, GL_ONE);

    /* Set up light1 */
    glEnable (GL_LIGHTING);
    glEnable (GL_LIGHT1);
    glLightfv (GL_LIGHT1, GL_POSITION, pos);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, white);
    glLightfv (GL_LIGHT1, GL_SPECULAR, white);

    /* ambient and diffuse will track glColor */
    glEnable (GL_COLOR_MATERIAL);
    glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glMaterialf (GL_FRONT, GL_SHININESS, 30.f);
    glMaterialfv (GL_FRONT, GL_SPECULAR, white);
    
    /* Set up texture coord generation */
    {
        /* Use Z plane for T generation because the cylinder lies along the Z axis */
        float tPlane[4] = {0., 0., 1., 0.};
        glTexGenfv (GL_T, GL_OBJECT_PLANE, tPlane);
    }
    glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glEnable (GL_TEXTURE_GEN_S);
    glEnable (GL_TEXTURE_GEN_T);
    
    /* prepare to use two texture objects */
    glGenTextures (2, textures);
    
    /* init first texture object, a red & black checkerboard, ho-hum */
    glBindTexture (GL_TEXTURE_2D, textures[0]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    {
#define TDIM 32
        unsigned char *pixels;
        pixels = (unsigned char *) malloc (TDIM*TDIM*4);
        initTextureMap (TDIM, TDIM, pixels);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, TDIM, TDIM,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        free (pixels);
    }

    /* Second texture is a 2x2 white texture map for the sdpecular pass of TMMODE_TWOPASS */
    glBindTexture (GL_TEXTURE_2D, textures[1]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    {
        unsigned char pixels[2*2*4];
        memset (pixels, 0xff, 2*2*4);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 2, 2,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    }
    
    /* Leave first texture bound */
    glBindTexture (GL_TEXTURE_2D, textures[0]);

    myQuad = gluNewQuadric ();

    glutDisplayFunc (display); 
    glutReshapeFunc (reshape);
    glutTimerFunc (TIMER, timer, 0);

    int value;
   
    GLfloat black[4] = {0., 0., 0., 0.};


    if (value == 99) {
        exit (0);
    }

    if (value < USE_ALPHA_TEST)
        tmMode = value;

        glEnable (GL_TEXTURE_2D);
        glLightfv (GL_LIGHT1, GL_SPECULAR, white);
#ifdef GL_HP_texture_lighting
        glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_LIGHTING_MODE_HP,
GL_TEXTURE_POST_SPECULAR_HP);
#endif /* GL_HP_texture_lighting */


 
//    mainMenu = glutCreateMenu (cbMainMenu);
    glutAddMenuEntry ("No texture mapping", TMMODE_NONE);
    glutAddMenuEntry ("Single pass", TMMODE_SINGLE);
    glutAddMenuEntry ("Two pass", TMMODE_TWOPASS);
#ifdef GL_HP_texture_lighting
    glutAddMenuEntry ("HP pre-specular", TMMODE_HPEXT);
#endif /* GL_HP_texture_lighting */
    glutAddMenuEntry ("Toggle alpha test", USE_ALPHA_TEST);
    glutAddMenuEntry ("Quit", 99);
    glutAttachMenu (GLUT_RIGHT_BUTTON);
    tmMode = TMMODE_NONE;
}

int main(int argc, char** argv)
{
    /* Pretty much standard GLUT init code sequence... */
    glutInit (&argc,argv);
    glutInitDisplayMode (GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE); 
    glutInitWindowSize (300,300);
    glutInitWindowPosition (0,0); 
    glutCreateWindow ("specular lighting and texture mapping");

    init ();

    glutMainLoop ();
    
}

