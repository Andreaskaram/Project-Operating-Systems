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
        echo
        echo "Passenger details:"
        echo "$results" | awk -F',' '{printf "Code: %s\nFull Name: %s\nAge: %s\nCountry: %s\nStatus: %s\nRescued: %s\n\n", $1, $2, $3, $4, $5, $6}'
    fi
}

update_passenger(){

    record=$(grep -i "$argname" $filename)

    if [[ -z $record ]]; then
        echo "No passenger found with the identifier '$argname'."
        return
    fi

    echo "Passenger found:"
    echo "$record" | awk -F',' '{printf "Code: %s\nFull Name: %s\nAge: %s\nCountry: %s\nStatus: %s\nRescued: %s\n\n", $1, $2, $3, $4, $5, $6}'

    case $field in
        fullname)
            sed -i '' "/$identifier/s/^\([^,]*,\)[^,]*/\1$argnewdata/" passengers.csv
            ;;
        age)
            sed -i '' "/$identifier/s/^\([^,]*,[^,]*,\)[^,]*/\1$argnewdata/" passengers.csv
            ;;
        country)
            sed -i '' "/$identifier/s/^\([^,]*,[^,]*,[^,]*,\)[^,]*/\1$argnewdata/" passengers.csv
            ;;
        status)
            sed -i '' "/$identifier/s/^\([^,]*,[^,]*,[^,]*,[^,]*,\)[^,]*/\1$argnewdata/" passengers.csv
            ;;
        rescued)
            sed -i '' "/$identifier/s/^\([^,]*,[^,]*,[^,]*,[^,]*,[^,]*,\)[^,]*/\1$argnewdata/" passengers.csv
            ;;
        record)
            sed -i '' "/$identifier/c$new_value" passengers.csv
            ;;
        *)
            echo "Invalid field."
            return
            ;;
    esac
}

argumentHandler(){
    echo $#
    if [[ $# -eq 0 ]]; then
        return
    elif [[ $# -eq 1 && "$1" == "reports" ]]; then
        echo "Reports generated"
    else
        local inNewdataField=false
        # Iterate through the command-line arguments
        for arg in "$@"; do
            if [[ "$inNewdataField" == false ]]; then
                # Check for any keyword of the form 'XXXXXX:'
                if [[ "$arg" =~ ^[a-zA-Z0-9_]+:$ ]]; then
                    inNewdataField=true
                    selectedField="${arg%:}"  # Store the current section name without the colon
                else
                # Append to the name if we are not in a section
                    if [[ -z "$argname" ]]; then
                        argname="$arg"
                    else
                        argname="$argname $arg"
                    fi
                fi
            else
            # After encountering a keyword like 'XXXXXX:', everything becomes newdata
            argnewdata="$argnewdata $arg"
            fi
        done
        echo "name: $argname"
        echo "data field selected: $selectedField"
        echo $argnewdata
    fi
}


argumentHandler "$@"
update_passenger

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


