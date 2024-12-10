#!/bin/bash

# Function to insert data into passengers.csv
insert_data() {
    read -p "Enter the filename (including path) to read data from (leave empty for manual entry): " filename

    if [[ -z $filename ]]; then
        echo "No filename provided. Switching to manual data entry."
        # Call manual entry subroutine
        echo "Enter passenger details in the format:"
        echo "[code],[fullname],[age],[country],[status (Passenger/Crew)],[rescued (Yes/No)]"
        echo "Type 'done' when you are finished entering data."
        
        filename="passengers.csv"
        
        # Open or create passengers.csv to append new data
        if [[ ! -f passengers.csv ]]; then
            touch passengers.csv
        fi

        while true; do
            read -p "Enter data: " data
            if [[ $data == "done" ]]; then
                break
            fi
            # Validate the data format
            if [[ $data =~ ^[0-9]+,.*,[0-9]+,.*,(Passenger|Crew),(Yes|No)$ ]]; then
                echo "$data" >> passengers.csv
            else
                echo "Invalid format. Please try again."
            fi
        done
        echo "Data saved to passengers.csv"
    else
        if [[ -f $filename ]]; then
            echo "Reading data from $filename..."
            echo "Data loaded successfully from $filename."
        else
            echo "File not found. Switching to manual data entry."
            insert_data # Recursive call for manual entry if file is invalid
        fi
    fi
}

# Function to search for a passenger by name
search_passenger() {
    if [[ ! -f $filename ]]; then
        echo "Error: passengers.csv not found."
        return
    fi

    read -p "Enter first name or last name to search: " name

    # Use grep to find the matching rows
    results=$(grep -i "$name" $filename)

    if [[ -z $results ]]; then
        echo "No passenger found with the name '$name'."
    else
        echo "Passenger details:"
        echo "$results" | awk -F',' '{printf "Code: %s\nFull Name: %s\nAge: %s\nCountry: %s\nStatus: %s\nRescued: %s\n\n", $1, $2, $3, $4, $5, $6}'
    fi
}
update_passenger(){
    echo    
}

if [ $# -lt 1 ]; then
    echo "Usage: $0 <name>"
    exit 1
fi

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


