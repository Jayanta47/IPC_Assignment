#!/bin/bash 

for i in {1..10}; do 
    grep -i "passenger $i " output.txt > "{$i}_chk.txt";
    grep -i "passenger $i " output2.txt > "{$i}_chk2.txt";

    kompare "{$i}_chk.txt" "{$i}_chk2.txt"

    rm -rf "{$i}_chk2.txt" "{$i}_chk.txt"
done