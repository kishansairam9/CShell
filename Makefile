main.out: main.c ls.o prompt.o cd.o echo.o pinfo.o run_command.o signal_handlers.o jobs.o itoa.o cronjob.o
	gcc -g main.c ls.o prompt.o cd.o echo.o pinfo.o run_command.o signal_handlers.o jobs.o itoa.o cronjob.o -L/usr/include -lreadline -o main.out

prompt.o: prompt.c
	gcc -c prompt.c

ls.o: ls.c
	gcc -c ls.c

cd.o: cd.c
	gcc -c cd.c

echo.o: echo.c
	gcc -c echo.c 

pinfo.o: pinfo.c
	gcc -c pinfo.c

run_command.o: run_command.c
	gcc -c run_command.c

signal_handlers.o: signal_handlers.c
	gcc -c signal_handlers.c

jobs.o: jobs.c
	gcc -c jobs.c

itoa.o: itoa.c
	gcc -c itoa.c

cronjob.o: cronjob.c
	gcc -c cronjob.c

clean:
	rm main.out ls.o prompt.o cd.o echo.o pinfo.o run_command.o jobs.o itoa.o cronjob.o
