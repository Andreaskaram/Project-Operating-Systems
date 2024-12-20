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

    read -p "Enter passenger code or fullname to search (leave empty to skip):" name

    if [[ -z $name ]]; then
        return
    fi

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
    elif [[ $# -ge 3 ]]; then
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
    else
        echo "Invalid arguments given"
    fi
}

display_file(){

    read -p "Type anything to print data (spacebar to see more - q to Quit) or leave empty to skip: " choice

    if [[ -z $choice ]]; then
        return
    fi
    cat "$filename" | less
}

generate_reports() {

    #echo "Processing data from $file_to_process"
    echo "Processing data from $filename"

    # Clear previous files to avoid appending data
    > ages.txt
    > percentages.txt
    > avg.txt
    > rescued.txt

    # Generate ages_*.txt
    awk -F'[;,]' '{
        age = $3;
        gsub(/^[ \t]+|[ \t]+$/, "", age);  # Remove leading and trailing whitespace
        if (age ~ /^[0-9]+$/) {  # Check if age is a number
            if (age <= 18) {
                group = "ages 0-18";
            } else if (age >= 19 && age <= 35) {
                group = "ages 19-35";
            } else if (age >= 36 && age <= 50) {
                group = "ages 36-50";
            } else {
                group = "ages 51+";
            }
            data[group] = data[group] $0 "\n";  # Append the line to the respective group
        }
    } END {
        # Write grouped data to the output file
        for (g in data) {
            print g ":" > "ages.txt";
            print data[g] >> "ages.txt";
            print "" >> "ages.txt";  # Add a blank line between groups
        }
    }' "$filename"

    # Generate percentages.txt
    awk -F'[;,]' '{
        age = $3;
        gsub(/^[ \t]+|[ \t]+$/, "", age);  # Remove leading and trailing whitespace
        status = $6;
        gsub(/^[ \t]+|[ \t]+$/, "", status);  # Remove leading and trailing whitespace
        if (age ~ /^[0-9]+$/) {  # Check if age is a number
            if (age <= 18) group = "0-18";
            else if (age >= 19 && age <= 35) group = "19-35";
            else if (age >= 36 && age <= 50) group = "36-50";
            else group = "51+";

            if (status ~ /^[Yy]es$/) rescued[group]++;
            total[group]++;
        }
    } END {
        printf "Percentages of rescued:\n" > "percentages.txt";
        for (g in total) {
            if (total[g] > 0) {
                printf "ages %s: %.2f%%\n", g, (rescued[g] / total[g]) * 100 > "percentages.txt";
            } else {
                printf "%s: 0.00%%\n", g > "percentages.txt";
            }
        }
    }' "$filename"

    # Generate avg.txt
    awk -F'[;,]' '{
        category = $5;
        gsub(/^[ \t]+|[ \t]+$/, "", category);  # Remove leading and trailing whitespace
        age = $3;
        gsub(/^[ \t]+|[ \t]+$/, "", age);  # Remove leading and trailing whitespace
        if (age ~ /^[0-9]+$/) {  # Check if age is a number
            ages[category] += age;
            counts[category]++;
        }
    } END {
        printf "Average age per passenger status:\n" > "avg.txt";
        for (category in ages) {
            if (counts[category] > 0) {
                printf "%s: %.2f\n", category, ages[category] / counts[category] > "avg.txt";
            }
        }
    }' "$filename"

    # Generate rescued.txt
    echo "Rescued passengers:" > rescued.txt
    grep -E '[;,][Yy]es$' "$filename" >> rescued.txt

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
elif [[ $argflag -eq 1 ]]; then
    generate_reports
    echo "Reports generated"
fi

search_passenger

display_file
