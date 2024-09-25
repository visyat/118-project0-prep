# NOTE: Make sure you have your project files in the ./project directory
# Will run the autograder and place the results in ./results/results.json
run:
	docker pull eado0/sockets-are-cool
	docker run --rm \
		-v ./project:/autograder/submission \
		-v ./results:/autograder/results \
		sockets-are-cool \
		/autograder/run_autograder && cat results/results.json

# In case you want to run the autograder manually, use interactive
interactive:
	docker pull eado0/sockets-are-cool
	(docker ps | grep sockets-are-cool && \
	docker exec -it eado0/sockets-are-cool bash) || \
	docker run --rm --name sockets-are-cool -it \
		-v ./project:/autograder/submission \
		-v ./results:/autograder/results \
		eado0/sockets-are-cool bash


