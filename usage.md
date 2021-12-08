### generate and copy a password

	echo "secret_key" > key.txt
	./pm -kf key.txt -gp -l password_name -c | xclip

### copy saved password

	./pm -kf key.txt -l pass -c | xclip

### encrypt binary

	./pm -kf key.txt -i binary.enc -df binary -b

### decrypt binary

	./pm -kf key.txt -i binary.enc -o binary.dec -b

### print all passwords

    ./pm -kf key.txt
