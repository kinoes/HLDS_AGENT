DIRS = common cppsqlite3 json xpacket xthread
		
all:
	@for d in ${DIRS}; do \
		( /bin/echo "<< Change directory: `pwd`/$$d >>"; \
		cd $$d; \
		${MAKE} all; \
		/bin/echo " " \
		); done

touch:
	@for d in ${DIRS}; do \
        ( /bin/echo "<< Change directory: `pwd`/$$d >>"; \
            cd $$d; \
            ${MAKE} touch; \
            /bin/echo " " \
        ); done

install:
	@for d in ${DIRS}; do \
        ( /bin/echo "<< Change directory: `pwd`/$$d >>"; \
            cd $$d; \
            ${MAKE} install; \
            /bin/echo " " \
        ); done
clean:
	@for d in $(DIRS); do\
		( /bin/echo "<< Change directory `pwd`/$$d >>"; \
		  cd $$d; \
		  $(MAKE) clean; \
		  /bin/echo " " \
		); done
