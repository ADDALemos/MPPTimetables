for FILE in data/input/ITC-2019/*; do
	echo $FILE;
	f="`echo "$FILE"|cut -d. -f1`"; 
	f=$f".wcnf"
	echo $f;
	./timetabler $FILE -formula=1 -verbosity=0 -algorithm=6 -pb=0 -opt-allocation -opt-cons;
	gzip -c $f > $f".gz";
	echo "Done";
	done
