# Makefile

# Define the shell script filename
SCRIPT_NAME = processes_ipc.sh

# Default target
all: run_script

# Rule to run the shell script
run_script:
	chmod +x processes_functions.sh processes_ipc.sh
	./$(SCRIPT_NAME)

# Optional: clean up any generated files if needed
clean:
	rm -f *.out