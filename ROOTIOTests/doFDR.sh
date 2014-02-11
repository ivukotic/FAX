#!/bin/zsh
r=($(curl "http://ivukotic.web.cern.ch/ivukotic/FDR/getTask.asp?SITE=$PANDA_SITE_NAME"))
echo $r
testID=$r[1]
name=$r[2]
testtype=$r[3]
server=$r[4]
address=$r[5]
files=$r[6]
timeout=$r[7]

echo "testID: $testID"
echo "name:$name "
echo "testtype:$testtype "
echo "server:$server "
echo "address:$address "
echo "files:$files "
echo "timeout:$timeout "



if [[ $testtype == 'FAXcopy' ]]; then
    
    for (( i=1; i<=$files; i++ ))
    do
        inpFile[i]=$r[8+($i-1)*2]
        inpFile[i]="${inpFile[i]/.MWT2./.$server.}"
        inpSize[i]=$r[9+($i-1)*2]
        echo "To do xrdcp of ${inpFile[i]} ${inpSize[i]}"
        
        echo "\`which time\` -f \"COPYTIME=%e\\nEXITSTATUS=%x\" --append -o logfile_$i xrdcp -f -np root://$address//${inpFile[i]} -> /dev/null 2>&1" >> toExecute.sh
        echo "echo BYTES=${inpSize[i]} >>logfile_$i" >> toExecute.sh    
    done
    
    chmod +x toExecute.sh
    ./toExecute.sh
    
    sumtime=0
    sumbytes=0
    succ=0
    echo "results:"
    
    for (( i=1; i<=$files; i++ ))
    do
        cat logfile_$i
        
        var1=$(grep "EXITSTATUS" logfile_$i)
        if [[ -z "$var1" ]] continue
        ES=$(echo $var1 | cut -f2 -d=)
        if [[ $ES > 0 ]] continue
            
        var1=$(grep "COPYTIME" logfile_$i)
        if [[ -z "$var1" ]] continue
        CT=$(echo $var1 | cut -f2 -d=)
        succ=$(($succ+1))
        sumtime=$(($sumtime+$CT))
        
        var1=$(grep "BYTES" logfile_$i)
        if [[ -z "$var1" ]] continue
        BR=$(echo $var1 | cut -f2 -d=)
        sumbytes=$((sumbytes+$BR))
        
    done
    
    report=""
    mbps=0
    if [[ $succ -eq $files ]]; then
        report="OK " 
    else
        report="Some%20copies%20failed.%20$succ%20were%20OK. "
    fi
    
    if [[ $sumtime > 0 ]]; then 
        mbps=$(($sumbytes/1024/1024/$sumtime))
    else
        echo "dont want to divide by 0 "
    fi
    
    echo "report: $report"
    echo "mbps: $mbps"
    
    curl "http://ivukotic.web.cern.ch/ivukotic/FDR/addResult.asp?testid=$testID&sum=$sumtime&mbps=$mbps&evps=0&pandaid=$PandaID&report=$report"
    
fi



if [[ $testtype == 'read10pc' ]]; then
    
    for (( i=1; i<=$files; i++ ))
    do
        inpFile[i]=$r[8+($i-1)*2]
        inpFile[i]="${inpFile[i]/.MWT2./.$server.}"
        inpSize[i]=$r[9+($i-1)*2]
        echo "To do 10pc read of ${inpFile[i]} ${inpSize[i]}"
        echo "./readDirect root://$address//${inpFile[i]} physics 10 30 > logfile_$i 2>&1 \n" >> toExecute.
    done
    
    chmod +x toExecute.sh
    ./toExecute.sh

    sum=0
    succ=0
    rootbytesread=0
    eventsread=0  
    for (( i=1; i<=$files; i++ ))
    do
        var1=$(grep "WALLTIME" logfile_$i)
        if [[ -z "$var1" ]] continue
        WT=$(echo $var1 | cut -f2 -d=)
        succ=$(($succ+1))
        sum=$(($sum+$WT))

        var1=$(grep "ROOTBYTESREAD" logfile_$i)
        BR=$(echo $var1 | cut -f2 -d=)
        rootbytesread=$(($rootbytesread+$BR))

        var1=$(grep "EVENTS" logfile_$i)
        EV=$(echo $var1 | cut -f2 -d=)
        eventsread=$(($eventsread+$EV))
    done

    report=""
    mbps=0
    evps=0

    if [[ $succ -eq $files ]]; then
        report="OK " 
    else
        report="Some reads failed. $succ were OK. "
    fi

    if [[ $sum > 0 ]]; then 
        mbps=$(($rootbytesread/1024/1024/$sum))
        evps=$(($eventsread/$sum))
    else
        echo "dont want to divide by 0 "
    fi

    echo "report: $report"
    echo "mbps: $mbps"
    echo "evps: $evps"


    curl "http://ivukotic.web.cern.ch/ivukotic/FDR/addResult.asp?testid=$testID&sum=$sum&mbps=$mbps&evps=$evps&pandaid=$PandaID&report=$report"
    
fi
