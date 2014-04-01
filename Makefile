CFLAGS = `pkg-config --cflags --libs sdl2`
LIB = -lglut -lGLU -lGL -lGLEW 

run: lab5.cpp
	g++ lab5.cpp -g $(LIB) $(CFLAGS) -o Lab5
        
clean: 
	rm -f *.out *~ run
