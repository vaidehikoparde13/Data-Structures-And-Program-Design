# Data-Structures-And-Program-Design
This repository consists of the mini-projects submitted as part of the Data Structures and Program Design course of second year, B.Tech in Computer Science and Engineering.

# DSPD_I_Assignment1_Structures
Using structures in C a program has been designed to perform various operations on stock related data. The data has been fetched from multiple (one for each day's data) .csv files, using File Handling in C.
The structure consists of 5 members wiz Open, High, Low and Close price of stock and volume. 
Functions provided include:
1) To find % change in the price of a stock from start to end date specified by the user.
2) To sort given stock data based on volume, and for equal volume based on close prices.
3) Find profit/loss of two or more stocks on based on the day they were bought and the day they were sold. 
4) To find the highest and lowest performing stock for a specified duration.
5) List all stock as per increasing the overall performance, for the specified duration.
6) And find average of average values of all stocks from start to end dates given by the user.

# DSPD_II_Assignment1_LinkedLists
This is a program for implementing an in-air flight-management platform of an air-traffic control (ATC) system, which maintains a digital dashboard for the in-air flights (the flights that have been departed but not yet arrived). 
The digital dashboard contains a linked list based database of in-air flights bucketed in 60 minutes intervals based on the expected time of arrival (ETA) of the flights. 
The system maintains a database of active buckets (the buckets which have at least one flight) sorted by the ETA intervals. 
Every bucket maintains the bucket ID (an integer), the beginning of the ETA interval, the end of the ETA interval, and a database of flight-plans. A flight-plan contains the flight ID (an integer), the actual departure time, and the ETA. 
In a bucket, the flight-plans are maintained as a database using a linked list sorted by the actual departure time of the flight.
Functions provided include:
1) Insertion, deletion and search of flight plans.
2) Display all data.
3) Rearrange the buckets according to the given time.

