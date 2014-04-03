#include <stdlib.h>
#include <string.h>
#include "GL/glut.h"

#define TIMER 33

GLUquadric *myQuad;

static float xrot;

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
    /*for the checkerboard */
    GLfloat white[4] = {1., 1., 1., 1.};
    GLfloat black[4] = {0., 0., 0., 0.};
    
    xrot = 0;

    glClearColor (.3, .3, .3, 0.);
    glColor4f (.5, .5, .5, 1.); /* cylinder is grey by default */
    glDisable (GL_DITHER);
    glEnable (GL_DEPTH_TEST);

    /* Set up alpha test */
    glAlphaFunc (GL_NOTEQUAL, 0);
    
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
    
    /* init first texture object and make the red & black checkerboard */
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
    
    glBindTexture (GL_TEXTURE_2D, textures[0]);

    myQuad = gluNewQuadric ();

    glutDisplayFunc (display); 
    glutReshapeFunc (reshape);
    glutTimerFunc (TIMER, timer, 0);

    int value;

    if (value == 99) {
        exit (0);
    }

    if (value < USE_ALPHA_TEST)
        tmMode = value;

        glEnable (GL_TEXTURE_2D);
        glLightfv (GL_LIGHT1, GL_SPECULAR, white);

        glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_LIGHTING_MODE_HP, GL_TEXTURE_POST_SPECULAR_HP);
}

int main(int argc, char** argv)
{
    /* Pretty much standard GLUT init code sequence... */
    glutInit (&argc,argv);
    glutInitDisplayMode (GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE); 
    glutInitWindowSize (300,300);
    glutInitWindowPosition (0,0); 
    glutCreateWindow ("Lab 5");

    init ();

    glutMainLoop ();
    
}

