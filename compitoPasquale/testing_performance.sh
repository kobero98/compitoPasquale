#! /bin/bash

#inserire in un file di configurazione
#-------------------------------------
DAT_FOLDER="dat"
PLOT_FOLDER="plot"
OUT_FOLDER="dat-aggragate"
PROGRAM_FOLDER="program"
NRUN="0" # 1 2 3 4 5 6 7 8 9"
BENCHMARK_FILES="input_c1.txt/check_c1.txt input_c2.txt/check_c2.txt"
TEST_DURATION="10"
MAX_RETRY="5"
#--------------------------------------
mkdir -p $OUT_FOLDER
mkdir -p $PLOT_FOLDER
mkdir -p $DAT_FOLDER


exec_test ()
{
        N=0
        echo $cmd
        while [ ! -f $nome_output ] || [ $(diff $check $nome_output -y --suppress-common-lines ) -eq 0 ]
        do
                echo $N
                $cmd
                if test $N -ge $MAX_RETRY ; then echo break; break; fi
                N=$(( N+1 ))
        done
}

#launch.sh
#python3 input_gen.py
for b in $BENCHMARK_FILES;do
    i=`python -c "print('$b'.split('/')[0])"`
    check=`python -c "print('$b'.split('/')[1])"`
    for r in $NRUN;do
        for p in $(ls $PROGRAM_FOLDER);do
            nome_output="check_output.txt"
            cmd=" $PROGRAM_FOLDER/$p $i $nome_output"
            { /usr/bin/time -f'elapsed:%E' $cmd ; } &> $DAT_FOLDER/$p-$i-$r.txt
            $cmd
            #exec_test $cmd $nome_output $check
        done
    done
done

line="PROGRAM"
file_output="$OUT_FOLDER/time.txt"
for p in $(ls $PROGRAM_FOLDER);do
    line="$line $p"
done
echo $line>$file_output


#aggragate.sh
for b in $BENCHMARK_FILES;do
    i=`python -c "print('$b'.split('/')[0])"`
    line="$i"
    for p in $(ls $PROGRAM_FOLDER);do
        sum=0
        count=0
        for r in $NRUN; do
                file_input="$DAT_FOLDER/$p-$i-$r.txt"
                echo "file:" $file_input
                cat $file_input
                val=`grep -e "elapsed:" $file_input`
                echo $val
                val=`python -c "print('$val'.replace('elapsed:', '').replace('ms',''))"`
                echo $val
                val=$(echo "$val" | awk -F '[:.]' '{print ($1*60 + $2)*1000000 + $3}')
                sum=`python -c "print($sum+$val)"`
                count=$(($count+1))
		    done
            avg=`python -c "print ($sum/$count)"`
            line="$line $avg"
    done
    echo $line>>$file_output

done
echo "ciao"
cat $file_output
