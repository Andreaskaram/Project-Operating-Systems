#!/bin/bash

# Import functions from processes_functions.sh
source ./processes_functions.sh

# Display welcome message
echo "======================================"
echo "Welcome to the Passenger Management System"
echo "======================================"

# Prompt the user to insert data
echo "Insert data into the application:"
insert_data

# Provide options for further actions
while true; do
    echo
    echo "Choose an option:"
    echo "1. Search for a passenger"
    echo "2. Exit"
    read -p "Enter your choice (1 or 2): " choice

    case $choice in
    1)
        search_passenger
        ;;
    2)
        echo "Exiting the program."
        exit 0
        ;;
    *)
        echo "Invalid choice. Please try again."
        ;;
    esac
done


