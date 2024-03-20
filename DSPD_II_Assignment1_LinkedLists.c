#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*---------------------------------------------------STRUCTURES----------------------------------------------------------*/

typedef struct time_Tag
{
    int hour;
    int min;
    int totalmin;
} time;

typedef struct flight_Tag
{
    int flightID;
    time DT; // Departure Time
    time ETA;
    struct flight_Tag *next;
} flightNode;

typedef struct bucket_Tag
{
    int bucketID;
    time beginETA;
    time endETA;
    flightNode *fhead;
    struct bucket_Tag *next;
} bucketNode;

/*----------------------------------------------------TIME FUNCTIONS-------------------------------------------------------*/

time addAdjustTime(time temp, int addMin)
{
    temp.min += addMin;
    if (temp.min >= 60)
    {
        temp.min = temp.min - 60;
        temp.hour = temp.hour + 1;
    }
    if (temp.hour >= 24)
    {
        temp.hour = temp.hour - 24;
    }
    temp.totalmin = (temp.hour * 60) + temp.min;
    return temp;
}

void printTime(time temp)
{
    printf(" %d:%d ", temp.hour, temp.min);
}

int timedeff(time A, time B)
{
    return fabs(A.totalmin - B.totalmin);
}

int maxtime(time A, time B)
{
    int ret_val;
    if (A.totalmin > B.totalmin)
    {
        ret_val = -1;
    }
    else if (A.totalmin < B.totalmin)
    {
        ret_val = 1;
    }
    else
    {
        ret_val = 0;
    }
    return ret_val;
}

time CalculateBucketBeginETA(time ETA, time curr)
{
    time beginETA;
    int diff = ETA.min - curr.min;
    beginETA.min = curr.min;
    if (diff >= 0)
    {
        beginETA.hour = ETA.hour;
    }
    else
    {
        beginETA.hour = ETA.hour - 1;
    }
    beginETA = addAdjustTime(beginETA, 0);
    return beginETA;
}

/*---------------------------------------------CREATE NODE FUNCTIONS------------------------------------------------------*/

flightNode *CreateFlightNode(int fID, time dt, time eta)
{
    flightNode *newNode = (flightNode *)malloc(sizeof(flightNode));
    newNode->flightID = fID;
    newNode->DT = dt;
    newNode->ETA = eta;
    newNode->next = NULL;
    return newNode;
}

bucketNode *CreateBucketNode(time beginETA)
{
    bucketNode *newNode = (bucketNode *)malloc(sizeof(bucketNode));
    static int bID = 1000;
    bID++;
    newNode->bucketID = bID;
    newNode->beginETA = addAdjustTime(beginETA, 0);
    if (newNode->beginETA.totalmin > 1380)
    {
        newNode->endETA.hour = 23;
        newNode->endETA.min = 59;
        newNode->endETA.totalmin = 1439;
    }
    else
    {
        newNode->endETA = addAdjustTime(beginETA, 59);
    }
    newNode->fhead = NULL;
    newNode->next = NULL;
    return newNode;
}
// if beginETA.totaltime > 1380 (23:00) then set endETA as 11:59

/*--------------------------------------------------PRINT FUNCTIONS---------------------------------------------------------*/

void printFlight(flightNode *flight)
{
    printf("ID-%d\t\tETA-", flight->flightID);
    printTime(flight->ETA);
    printf("\tDT-");
    printTime(flight->DT);
}

void printBucket(bucketNode *bucket)
{
    printf("ID: %d, ", bucket->bucketID);
    printf(" from ");
    printTime(bucket->beginETA);
    printf(" to ");
    printTime(bucket->endETA);
}

void printFlightPlan(bucketNode *bhead, int fID)
{
    int f = 0; // flag
    bucketNode *bptr;
    flightNode *fptr;
    bptr = bhead;
    fptr = NULL;
    if (bhead == NULL)
    {
        printf("Error, Bucket List is empty");
    }
    else
    {
        while (bptr != NULL && f == 0)
        {
            fptr = bptr->fhead;
            while (fptr != NULL && f == 0)
            {
                if (fptr->flightID == fID)
                {
                    f = 1;
                }
                else
                {
                    fptr = fptr->next;
                }
            }
            if (f == 0)
            {
                bptr = bptr->next;
            }
        }

        // if flight plan does not found
        if (f == 0)
        {
            printf("No such flight found\n");
        }
        else
        {
            printFlight(fptr);
        }
    }
}

void printAll(bucketNode *bhead)
{
    bucketNode *bptr;
    flightNode *fptr;
    bptr = bhead;
    fptr = NULL;
    if (bhead == NULL)
    {
        printf("Error, No bucket in the list\n");
    }

    while (bptr != NULL)
    {
        printf("\nBucket ");
        printBucket(bptr);
        printf("\nFlights: \n");
        fptr = bptr->fhead;
        while (fptr != NULL)
        {
            printFlight(fptr);
            fptr = fptr->next;
            printf("\n");
        }
        bptr = bptr->next;
    }
}

/*------------------------------------INSERT FLIGHT AND INSERT BUCKET FUNCTIONS----------------------------------------------*/

bucketNode *InsertFlight(bucketNode *bhead, flightNode *newNode, time curr)
{
    // time curr;
    if (bhead == NULL)
    {
        time beginETA = CalculateBucketBeginETA(newNode->ETA, curr);
        bhead = CreateBucketNode(beginETA);
        bhead->fhead = newNode;
    }
    else
    {
        bucketNode *bptr = bhead;
        bucketNode *bprev = NULL;
        while (bptr != NULL && bptr->beginETA.totalmin <= newNode->ETA.totalmin)
        {
            bprev = bptr;
            bptr = bptr->next;
        }

        if (bprev != NULL)
        {
            // if you find the correct bucket
            if (bprev->beginETA.totalmin <= newNode->ETA.totalmin && newNode->ETA.totalmin <= bprev->endETA.totalmin)
            {
                flightNode *fptr = bprev->fhead;
                flightNode *fprev = NULL;
                if (fptr == NULL) // probably wont occur as you dont maintain empty buckets
                {
                    bprev->fhead = newNode;
                }
                else
                {
                    while (fptr != NULL && fptr->DT.totalmin <= newNode->DT.totalmin)
                    {
                        fprev = fptr;
                        fptr = fptr->next;
                    }
                    if (fprev == NULL)
                    {
                        newNode->next = fptr;
                        bprev->fhead = newNode;
                    }
                    else
                    {
                        newNode->next = fptr;
                        fprev->next = newNode;
                    }
                }
            }
            else
            {
                // if correct bucket not found, create bucket, insert it in the right place between bprev and bptr and then insert flight plan
                time beginETA = CalculateBucketBeginETA(newNode->ETA, curr);
                bucketNode *newbucket = CreateBucketNode(beginETA);
                newbucket->next = bptr;
                bprev->next = newbucket;
                newbucket->fhead = newNode;
            }
        }
        else
        {
            // bptr == bhead
            // you need to create a bucket at the start and insert the flight node there
            time beginETA = CalculateBucketBeginETA(newNode->ETA, curr);
            bucketNode *newbucket = CreateBucketNode(beginETA);
            newbucket->next = bptr;
            bhead = newbucket;
            newbucket->fhead = newNode;
        }
    }
    return bhead;
}

/*------------------------------------------------CANCEL A FLIGHT PLAN--------------------------------------------------------*/

bucketNode *Cancelflightplan(bucketNode *bhead, int fID)
{
    // we can search for the ETA of that flight using fID in the file and store it in time temp
    // time temp;
    int f = 0; // flag
    bucketNode *bptr, *bprev;
    flightNode *fptr, *fprev;
    bptr = bhead;
    bprev = NULL;
    fptr = fprev = NULL;
    if (bhead == NULL)
    {
        printf("Error, there is no bucket in the list");
    }
    else
    {
        // finding the flight plan
        while (bptr != NULL && f == 0)
        {
            fprev = fptr;
            fptr = bptr->fhead;
            while (fptr != NULL && f == 0)
            {
                if (fptr->flightID == fID)
                {
                    f = 1;
                }
                else
                {
                    fprev = fptr;
                    fptr = fptr->next;
                }
            }
            if (f == 0)
            {
                bprev = bptr;
                bptr = bptr->next;
            }
        }

        // if flight plan does not found
        if (f == 0)
        {
            printf("No such flight found\n");
        }

        else
        {
            // if the flight is at start of the flight plan list or it is the only flight plan in the flight plan list
            if (fprev == NULL)
            {
                bptr->fhead = fptr->next;
                free(fptr);
                if (bptr->fhead == NULL)
                {
                    // delete the bucket
                    if (bprev == NULL)
                    {
                        bhead = bptr->next;
                        free(bptr);
                    }
                    else
                    {
                        bprev->next = bptr->next;
                        free(bptr);
                    }
                }
            }

            // if the flight is at middle or end of the flight plan list
            else
            {
                fprev->next = fptr->next;
                free(fptr);
            }
            printf("\nFlight with id %d cancelled. \n\033[0;33m------------------------------New List-------------------------------------------------------\033[0m\n", fID);
            printAll(bhead);
        }
    }
    return bhead;
}

/*----------------------------------FILE HANDLING------------------------------------------------*/

void loadFlightsFromFile(const char *filename, bucketNode **bbhead, time curr)
{
    bucketNode *bhead = *bbhead;
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Unable to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    char line[100]; // Adjust the size accordingly
    while (fgets(line, sizeof(line), file) != NULL)
    {
        int flightID;
        int ETAHour, ETAMin, DTHour, DTMin;

        sscanf(line, "%d,%d:%d,%d:%d", &flightID, &ETAHour, &ETAMin, &DTHour, &DTMin);

        time eta = {ETAHour, ETAMin, 0};
        time dt = {DTHour, DTMin, 0};

        eta = addAdjustTime(eta, 0);
        dt = addAdjustTime(dt, 0);

        flightNode *newNode = CreateFlightNode(flightID, dt, eta);
        printf("\n");
        printFlight(newNode);
        bhead = InsertFlight(bhead, newNode, curr);
    }

    fclose(file);
    *bbhead = bhead;
}

/*----------------------------------REARRANGING ACCORDING TO CURRENT TIME---------------------------------------------*/

bucketNode *Rearrange(bucketNode *bhead, time curr)
{
    bucketNode *bptr = bhead;
    flightNode *fptr;

    // deleting all the nodes which are before the current time
    while (bptr != NULL && curr.totalmin > bhead->endETA.totalmin)
    {
        bhead = bhead->next;
        while (bptr->fhead != NULL)
        {
            fptr = bptr->fhead;
            bptr->fhead = bptr->fhead->next;
            free(fptr);
            fptr = NULL;
        }
        free(bptr);
        bptr = bhead;
    }

    while (bhead != NULL && bhead->fhead->ETA.totalmin < curr.totalmin)
    {
        fptr = bhead->fhead;
        bhead->fhead = bhead->fhead->next;
        free(fptr);
        fptr = NULL;
        if (bhead->fhead == NULL)
        {
            bptr = bhead;
            bhead = bhead->next;
            free(bptr);
            bptr = NULL;
        }
    }
    // deletion ends now bhead is at the bucket with the first flight which fits into our interval

    bucketNode *current, *next_ptr, *nbhead;
    flightNode *temp;
    current = next_ptr = NULL;
    temp = NULL;

    bptr = bhead;
    fptr = NULL;
    nbhead = NULL;
    while (bhead != NULL)
    {
        fptr = bhead->fhead;
        while (fptr != NULL)
        {
            time beginETA = CalculateBucketBeginETA(fptr->ETA, curr);
            if (current == NULL)
            {
                nbhead = CreateBucketNode(beginETA);
                nbhead->fhead = fptr;
                bhead->fhead = fptr->next;
                fptr->next = NULL;
                current = nbhead;
            }
            else
            {
                if (fptr->ETA.totalmin <= current->endETA.totalmin)
                {
                    // we'll insert it in the current list
                    temp = current->fhead;
                    while (temp->next != NULL)
                    {
                        temp = temp->next;
                    }
                    temp->next = fptr;
                    bhead->fhead = fptr->next;
                    fptr->next = NULL;
                }
                else if (next_ptr != NULL && fptr->ETA.totalmin <= next_ptr->endETA.totalmin)
                {
                    // we'll insert it in the next list
                    temp = next_ptr->fhead;
                    while (temp->next != NULL)
                    {
                        temp = temp->next;
                    }
                    temp->next = fptr;
                    bhead->fhead = fptr->next;
                    fptr->next = NULL;
                }
                else if (next_ptr == NULL)
                {
                    // will have to create a new bucket
                    next_ptr = CreateBucketNode(beginETA);
                    current->next = next_ptr;
                    next_ptr->fhead = fptr;
                    bhead->fhead = fptr->next;
                    fptr->next = NULL;
                }
                else // if it does not fit into the current as well as next
                {
                    current = next_ptr;
                    // create a new bucket and insert it there
                    next_ptr = CreateBucketNode(beginETA);
                    current->next = next_ptr;
                    next_ptr->fhead = fptr;
                    bhead->fhead = fptr->next;
                    fptr->next = NULL;
                }
            }
            fptr = bhead->fhead;
        }
        bptr = bhead;
        bhead = bhead->next;
        free(bptr);
        bptr = NULL;
    }
    return nbhead;
}

/*----------------------------------------MAIN FUNCTION------------------------------------------------*/

int main()
{
    bucketNode *bucketHead = NULL;
    char filename[20] = "FlightData.csv";
    int n, id;
    time currTime = {0, 0, 0};
    time prevTime = {0, 0, 0};
    currTime = addAdjustTime(currTime, 0);
    prevTime = addAdjustTime(prevTime, 0);
    printf("\033[0;34m");
    printf("\n------------------------------Loading Today's Flights----------------------------------------------\n");
    loadFlightsFromFile(filename, &bucketHead, currTime);
    do
    {
        printf("\033[0;36m");
        printf("\n\n------------------------------Program Menu - Current Time is %d:%d--------------------------------------\n", currTime.hour, currTime.min);
        printf("\nPress 1: Display all flights.");
        printf("\nPress 2: Display a particular flight.");
        printf("\nPress 3: Cancel a particular flight.");
        printf("\nPress 4: Rearrange flights according to current time.");
        printf("\nPress 5: Add flight.\n");
        printf("\nPress 6: Exit Program.\n");
        scanf("\n\n%d", &n);
        printf("\033[0;37m");
        if (n == 1)
        {
            printf("\n\033[0;33m------------------------------Today's Flights------------------------------\033[0m\n\n");
            printAll(bucketHead);
        }
        else if (n == 2)
        {
            printf("\n\nEnter the ID of the flight : ");
            scanf("%d", &id);
            printFlightPlan(bucketHead, id);
        }
        else if (n == 3)
        {
            printf("\nEnter the ID of the flight to be cancelled: ");
            scanf("%d", &id);
            bucketHead = Cancelflightplan(bucketHead, id);
        }
        else if (n == 4)
        {

            printf("\nTo rearrange, enter current time: ");
            do
            {
                do
                {
                    printf("\nHour: ");
                    scanf("%d", &currTime.hour);
                } while (currTime.hour > 23 && printf("\nEnter valid hour."));
                do
                {
                    printf("\nMin: ");
                    scanf("%d", &currTime.min);
                } while (currTime.min > 59 && printf("\nEnter valid min."));
                currTime = addAdjustTime(currTime, 0);
            } while (currTime.totalmin < prevTime.totalmin && printf("\nEnter present/ future time."));

            bucketHead = Rearrange(bucketHead, currTime);
            printf("\n\033[0;33m------------------------------After Rearranging------------------------------\033[0m\n\n");
            printAll(bucketHead);
        }
        else if (n == 5)
        {
            int flightID;
            printf("\nEnter flight ID:");
            scanf("%d", &flightID);
            time eta, dt;
            printf("\nEnter expected time of arrival: ");
            do
            {
                printf("\nHour: ");
                scanf("%d", &eta.hour);
            }
            while (eta.hour > 23 && printf("\nEnter valid hour."));
            do
            {
                printf("\nMin: ");
                scanf("%d", &eta.min);
            } while (eta.min > 59 && printf("\nEnter valid min."));
            eta = addAdjustTime(eta, 0);

            printf("\nEnter departure time: ");
            do
            {
                printf("\nHour: ");
                scanf("%d", &dt.hour);
            }
            while (dt.hour > 23 && printf("\nEnter valid hour."));
            do
            {
                printf("\nMin: ");
                scanf("%d", &dt.min);
            } while (dt.min > 59 && printf("\nEnter valid min."));
            dt = addAdjustTime(dt, 0);

            flightNode *newNode;
            newNode = CreateFlightNode(flightID, dt, eta);
            bucketHead = InsertFlight(bucketHead, newNode, currTime);
            printf("\nAdded flight with id %d", flightID);
            printf("\n-------------------------------Updated flight list -----------------------\n");
            printAll(bucketHead);
        }
        else if (n > 6 || n < 1)
        {
            printf("\nEnter valid number.");
        }
        prevTime = currTime;
    } while (n != 6);
    printf("\n\033[0;33m------------------------------Thank you!------------------------------\033[0m\n\n");
    return 0;
}
