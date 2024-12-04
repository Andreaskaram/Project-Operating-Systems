#!/bin/bash

# Function to append manually entered data to passengers.csv
enter_data_manually() {
    echo "Enter passenger details in the format:"
    echo "[code],[fullname],[age],[country],[status (Passenger/Crew)],[rescued (Yes/No)]"
    echo "Type 'done' when you are finished entering data."
    
    # Open or create passengers.csv to append new data
    if [[ ! -f passengers.csv ]]; then
        touch passengers.csv
    fi

    while true; do
        read -p "Enter data: " data
        if [[ $data == "done" ]]; then
            break
        fi
        echo "$data" >> passengers.csv
    done
    echo "Data saved to passengers.csv"
}

# Request filename from user
read -p "Enter the filename (including path) to read data from: " filename

if [[ -z $filename ]]; then
    echo "No filename provided. Switching to manual data entry."
    enter_data_manually
else
    if [[ -f $filename ]]; then
        echo "Reading data from $filename..."
        #cat "$filename"
    else
        echo "File not found. Switching to manual data entry."
        enter_data_manually
    fi
fi
