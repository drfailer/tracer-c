test: main.c
	gcc -o $@ $^ -I../tracer -Wl,-rpath,../build/lib/ -L../build/lib/ -ltracer

clean:
	rm -rf test
	rm -rf test.tr
