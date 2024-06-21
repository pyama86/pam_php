build:
	gcc -O0 -g3 `php-config --includes -libs` -fPIC -shared -o pam_php.so php.c -lpam -lphp -lc

install: build
	cp pam_php.so /usr/lib/aarch64-linux-gnu/security

docker:
	docker build -t pam_php .
	docker run --rm -v `pwd`:/work -w /work -it pam_php /bin/bash
