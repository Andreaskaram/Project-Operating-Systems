#!/bin/bash

# Function to insert data into passengers.csv
insert_data() {
    read -p "Enter the filename (including path) to read data from (leave empty for manual entry): " filename

    if [[ -z $filename ]]; then
        echo "No filename provided. Switching to manual data entry."
        # Call manual entry subroutine
        echo "Enter passenger details in the format:"
        echo "[code],[fullname],[age],[country],[status (Passenger/Crew)],[rescued (Yes/No)] (No spaces allowed)"
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
            echo
            echo "Data loaded successfully from $filename."
        else
            echo "File $filename not found. Please try again."
            insert_data # Recursive call for manual entry if file is invalid
        fi
    fi
}

# Function to search for a passenger by name
search_passenger() {
    if [[ ! -f $filename ]]; then
        echo "Error: $filename not found."
        return
    fi

    read -p "Enter passenger code or fullname to search: " name

    # Use grep to find the matching rows
    #results=$(grep -i "\b$name\b" $filename)
    results=$(awk -F',' -v name="$name" 'BEGIN {IGNORECASE = 1} ($1 == name || $2 == name) {print $0}' "$filename")

    if [[ -z $results ]]; then
        echo "No passenger found with the code/fullname '$name'!"
    else
        echo
        echo "Passenger details:"
        echo "$results" | awk -F',' '{printf "Code: %s\nFull Name: %s\nAge: %s\nCountry: %s\nStatus: %s\nRescued: %s\n\n", $1, $2, $3, $4, $5, $6}'
    fi
}

update_passenger() {
    results=$(awk -F',' -v name="$argname" 'BEGIN {IGNORECASE = 1} ($1 == name || $2 == name) {print $0}' "$filename")

    if [[ -z $results ]]; then
        echo "No passenger found with the identifier '$argname'."
        return
    fi

    echo "Passenger found:"
    echo "$results" | awk -F',' '{printf "Code: %s\nFull Name: %s\nAge: %s\nCountry: %s\nStatus: %s\nRescued: %s\n\n", $1, $2, $3, $4, $5, $6}'

    case $selectedField in
        fullname)
            sed -i '' "/^$argname,/ s/\([^,]*,\)[^,]*/\1$argnewdata/" "$filename"
            ;;
        age)
            sed -i '' "/^$argname,/ s/\([^,]*,[^,]*,\)[^,]*/\1$argnewdata/" "$filename"
            ;;
        country)
            sed -i '' "/^$argname,/ s/\([^,]*,[^,]*,[^,]*,\)[^,]*/\1$argnewdata/" "$filename"
            ;;
        status)
            sed -i '' "/^$argname,/ s/\([^,]*,[^,]*,[^,]*,[^,]*,\)[^,]*/\1$argnewdata/" "$filename"
            ;;
        rescued)
            sed -i '' "/^$argname,/ s/\([^,]*,[^,]*,[^,]*,[^,]*,[^,]*,\)[^,]*/\1$argnewdata/" "$filename"
            ;;
        record)
            if [[ $argnewdata =~  ^[0-9]+,.*,[0-9]+,.*,(Passenger|Crew),(Yes|No)$ ]]; then
                sed -i '' "/$argname/ s/.*/$argnewdata/" "$filename"
            else
                echo "Invalid format. Please try again. (No spaces allowed - Case sensitive)"
            fi
            ;;
        *)
            echo "Invalid field."
            return
            ;;
    esac

    echo "Update complete."
}

argumentHandler(){
    echo $#
    if [[ $# -eq 0 ]]; then
        return
    elif [[ $# -eq 1 && "$1" == "reports" ]]; then
        argflag=1
        echo "Reports generated"
    else
        argflag=2
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
            argnewdata=$(echo "$argnewdata" | sed 's/^[ \t]*//;s/[ \t]*$//')
            fi
        done
        echo "name: $argname"
        echo "data field selected: $selectedField"
        echo $argnewdata
    fi
}

display_file(){
    cat "$filename" | less
}

argflag=0
argumentHandler "$@"


# Display welcome message
echo "======================================"
echo "Welcome to the Passenger Management System"
echo "======================================"

# Prompt the user to insert data
echo "Insert data into the application:"
insert_data

if [[ $argflag -eq 2 ]]; then
    update_passenger
fi

# Provide options for further actions
while true; do
    echo
    echo "Choose an option:"
    echo "1. Search for a passenger"
    echo "2. Exit"
    echo "3. Print csv (space to show more - q to exit)"
    read -p "Enter your choice (1 or 2): " choice

    case $choice in
    1)
        search_passenger
        ;;
    2)
        echo "Exiting the program."
        exit 0
        ;;
    3)
        display_file
        ;;
    *)
        echo "Invalid choice. Please try again."
        ;;
    esac
done


