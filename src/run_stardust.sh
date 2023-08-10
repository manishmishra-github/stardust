#!/bin/bash

echo "\nRUNNING STARDUST WITH PRIORITY Q\n"

#make clean;
make

sudo xl destroy stardust
sudo xl create -c stardust.conf 2>&1 | tee logs.csv &


while true; do
    # Get the state of the 'stardust' program
    state=$(sudo xl list | grep "stardust" | awk '{print $5}')
    echo $state

    # Check if the state is '-b----'
    if [ "$state" == "-b----" ]; then
        # Send the SIGINT signal to the 'stardust' program
        #pkill stardust
        sed -i '1,35d' logs.csv

        sed -i '1i THREAD ID,PRIORITY,SCHEDULE COUNT,CREATION TIME,EXECUTION TIME,SCHEDULE INDEX' logs.csv
    
        scp logs.csv mm551@mm551.host.cs.st-andrews.ac.uk:/home/mm551/Documents/RESULTS/
        #sshpass -p 'password' scp -p 'Fury@180597' logs.csv mm551@mm551.host.cs.st-andrews.ac.uk:/home/mm551/Documents/RESULTS/
        
        # Exit the loop
        break
    fi

    # Sleep for a while before checking again
    sleep 5
done


sudo xl destroy stardust

#sed -i '1,35d' logs.csv

#sed -i '1i THREAD ID,PRIORITY,SCHEDULE COUNT,CREATION TIME,EXECUTION TIME,SCHEDULE INDEX' logs.csv

#scp -p'Fury@180597' logs.csv mm551@mm551.host.cs.st-andrews.ac.uk:/home/mm551/Documents/RESULTS/
