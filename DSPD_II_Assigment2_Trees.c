#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int hash[1000][2]; // 0->ETA, 1->DT

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
    int height;
    struct flight_Tag *left;
    struct flight_Tag *right;
    struct flight_Tag *subroot;
} flightNode;

typedef struct bucket_Tag
{
    int bucketID;
    time beginETA;
    time endETA;
    flightNode *froot;
    struct bucket_Tag *next;
} bucketNode;

/*----------------------------------------TIME AND CALCULATION FUNCTIONS-------------------------------------------------------*/

int max(int a, int b)
{
    return a > b ? a : b;
}

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
    newNode->height = 0;
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->subroot = NULL;
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
    newNode->froot = NULL;
    newNode->next = NULL;
    return newNode;
}

// if beginETA.totaltime > 1380 (23:00) then set endETA as 11:59

/*------------------------------------------------PRINT FUNCTIONS---------------------------------------------------------*/

void printFlight(flightNode *flight)
{
    printf("\033[0;37m");
    printf("\t\tID-%d\t\tETA-", flight->flightID);
    printTime(flight->ETA);
    printf("\tDT-");
    printTime(flight->DT);
    printf("\n");
    printf("\033[0;37m");
}

void printBucket(bucketNode *bucket)
{
    printf("\033[0;36m");
    printf("\tID: %d, ", bucket->bucketID);
    printf(" from ");
    printTime(bucket->beginETA);
    printf(" to ");
    printTime(bucket->endETA);
    printf("\n");
    printf("\033[0;37m");
}

void printInOrder(flightNode *root)
{
    if (root != NULL)
    {
        printInOrder(root->left);
        printFlight(root);
        printInOrder(root->subroot);
        printInOrder(root->right);
    }
}

void printInOrderWrite(flightNode *root, FILE* file)
{
    if (root != NULL)
    {
        printInOrderWrite(root->left, file);
        fprintf(file, "%d,%d:%d,%d:%d\n", root->flightID, root->ETA.hour, root->ETA.min, root->DT.hour, root->DT.min);
        printInOrderWrite(root->subroot, file);
        printInOrderWrite(root->right, file);
    }
}

void printAll(bucketNode *bhead)
{
    bucketNode *bptr;
    bptr = bhead;
    if (bhead == NULL)
    {
        printf("Error, No bucket in the list\n");
    }
    while (bptr != NULL)
    {
        printf("\n\033[0;36mBucket: \033[0m");
        printBucket(bptr);
        printf("\n\033[0;32mFlights: \033[0\n");
        printInOrder(bptr->froot);
        printf("\n");
        bptr = bptr->next;
    }
}

/*------------------------------------------------TREE FUNCTIONS---------------------------------------------------------*/

int Height(flightNode *root)
{
    int height = -1;
    if (root != NULL)
    {
        if ((root->left == NULL) && (root->right == NULL))
        {
            height = 0;
        }
        else
        {
            int h1 = Height(root->left);
            int h2 = Height(root->right);
            height = 1 + max(h1, h2);
        }
    }
    return height;
}

int getBalanceFactor(flightNode *root)
{
    int ret_val = 0;
    if (root != NULL)
    {
        int h1 = Height(root->left);
        int h2 = Height(root->right);
        ret_val = h1 - h2;
    }
    return ret_val;
}

flightNode *RotateRight(flightNode *y)
{
    flightNode *x = y->left;
    y->left = x->right;
    x->right = y;

    y->height = Height(y);
    x->height = Height(x);

    return x;
}

flightNode *RotateLeft(flightNode *x)
{
    flightNode *y = x->right;
    x->right = y->left;
    y->left = x;

    y->height = Height(y);
    x->height = Height(x);

    return y;
}

flightNode *InsertSubNode(flightNode *root, flightNode *newNode)
{
    int d = newNode->flightID;
    if (root == NULL)
    {
        root = newNode;
    }
    else
    {
        if (d < root->flightID)
        {
            root->left = InsertSubNode(root->left, newNode);
        }
        else
        {
            root->right = InsertSubNode(root->right, newNode);
        }

        root->height = Height(root);

        int bf = getBalanceFactor(root);

        if (bf > 1 && d < root->left->flightID) // LL
        {
            root = RotateRight(root);
        }
        else if (bf < -1 && d > root->right->flightID) // RR
        {
            root = RotateLeft(root);
        }
        else if (bf > 1 && d > root->left->flightID) // LR
        {
            root->left = RotateLeft(root->left);
            root = RotateRight(root);
        }
        else if (bf < -1 && d < root->right->flightID)
        {
            root->right = RotateRight(root->right);
            root = RotateLeft(root);
        }
    }
    return root;
}

flightNode *InsertNode(flightNode *root, flightNode *newNode)
{
    int d = newNode->DT.totalmin;
    if (root == NULL)
    {
        root = newNode;
    }
    else
    {
        if (d < root->DT.totalmin)
        {
            root->left = InsertNode(root->left, newNode);
        }
        else if (d > root->DT.totalmin)
        {
            root->right = InsertNode(root->right, newNode);
        }
        else
        {
            root->subroot = InsertSubNode(root->subroot, newNode);
        }

        root->height = Height(root);

        int bf = getBalanceFactor(root);

        if (bf > 1 && d < root->left->DT.totalmin) // LL
        {
            root = RotateRight(root);
        }
        else if (bf < -1 && d > root->right->DT.totalmin) // RR
        {
            root = RotateLeft(root);
        }
        else if (bf > 1 && d > root->left->DT.totalmin) // LR
        {
            root->left = RotateLeft(root->left);
            root = RotateRight(root);
        }
        else if (bf < -1 && d < root->right->DT.totalmin)
        {
            root->right = RotateRight(root->right);
            root = RotateLeft(root);
        }
    }
    return root;
}

/*------------------------------------------------DELETE FUNCTIONS---------------------------------------------------------*/

flightNode *DeleteNode(flightNode *nptr)
{
    printf("\n\nDetails of the flight which is to be deleted: \n");
    printFlight(nptr);
    printf("\n------------------------------------------------------------\n");
    flightNode *ptr, *prev, *ret_val = NULL;
    if (nptr == NULL)
    {
        printf("Error");
    }
    else if (nptr->left == NULL) // either both will be null i.e. zero child or if right exist then 1 child
    {
        ret_val = nptr->right;
        free(nptr);
    }
    else if (nptr->right == NULL) // there is left child
    {
        ret_val = nptr->left;
        free(nptr);
    }
    else // approach a i.e. to attacch a part of tree to the another part
    {
        ptr = nptr->left;
        while (ptr->right != NULL)
        {
            ptr = ptr->right;
        }
        ptr->right = nptr->right;
        ret_val = nptr->left;
        free(nptr);
    }
    return ret_val;
}

flightNode *DeleteTreeNode(flightNode *root, int dt, int fID)

{
    flightNode *ret_val = root;

    if (root == NULL)
    {
        printf("Error\n");
    }
    else
    {
        if (dt < root->DT.totalmin)
        {
            root->left = DeleteTreeNode(root->left, dt, fID);
        }
        else if (dt > root->DT.totalmin)
        {
            root->right = DeleteTreeNode(root->right, dt, fID);
        }
        else if (dt == root->DT.totalmin && fID == root->flightID)
        {
            root = DeleteNode(root);
        }
        else if (dt == root->DT.totalmin && fID != root->flightID)
        {
            root = DeleteTreeNode(root->subroot, dt, fID);
        }

        if (root != NULL)
        {
            root->height = Height(root);

            int bf = getBalanceFactor(root);

            if (bf > 1 && dt < root->left->DT.totalmin) // LL
            {
                root = RotateRight(root);
            }
            else if (bf < -1 && dt > root->right->DT.totalmin) // RR
            {
                root = RotateLeft(root);
            }
            else if (bf > 1 && dt > root->left->DT.totalmin) // LR
            {
                root->left = RotateLeft(root->left);
                root = RotateRight(root);
            }
            else if (bf < -1 && dt < root->right->DT.totalmin)
            {
                root->right = RotateRight(root->right);
                root = RotateLeft(root);
            }

            if (root->left != NULL)
                root->left->height = Height(root->left);
            if (root->right != NULL)
                root->right->height = Height(root->right);
        }
    }
    return root;
}

bucketNode *CancelFlightPlan(bucketNode *bhead, int fID)
{
    bucketNode *bprev, *bptr;
    bprev = bptr = bhead;

    if (bptr == NULL)
    {
        printf("\nNo data available");
    }
    else
    {
        int eta_tt = hash[fID][0];
        int dt_tt = hash[fID][1];
        while (bptr != NULL && eta_tt > bptr->endETA.totalmin)
        {
            bprev = bptr;
            bptr = bptr->next;
        }

        if (bptr == NULL)
        {
            printf("\nFlight not found");
        }
        else
        {
            if (bptr->beginETA.totalmin <= eta_tt && eta_tt <= bptr->endETA.totalmin)
            {
                bptr->froot = DeleteTreeNode(bptr->froot, dt_tt, fID);

                // if the deleted node was the only flight in the bucket
                if (bprev != NULL && bptr->froot == NULL)
                {
                    bprev->next = bptr->next;
                    free(bptr);
                    bptr = bprev->next;
                }
                // else if the deleted node was the only flight in the bucket and the bucket was the first bucket
                else if (bprev == NULL && bptr->froot == NULL)
                {
                    bhead = bptr->next;
                    free(bptr);
                }
                printf("\nFlight with ID %d cancelled\n", fID);
                printf("\n\nUpdated List\n");
                printAll(bhead);
            }
            else
            {
                printf("\nFlight not found");
            }
        }
    }
    return bhead;
}

/*--------------------------------------------INSERT FLIGHT INSIDE BUCKET---------------------------------------*/

bucketNode *InsertFlightInBucket(bucketNode *bhead, flightNode *newNode, time curr)
{
    if (bhead == NULL)
    {
        time beginETA = CalculateBucketBeginETA(newNode->ETA, curr);
        bhead = CreateBucketNode(beginETA);
        bhead->froot = InsertNode(bhead->froot, newNode);
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
                bprev->froot = InsertNode(bprev->froot, newNode);
            }
            else
            {
                // if correct bucket not found, create bucket, insert it in the right place between bprev and bptr and then insert flight plan
                time beginETA = CalculateBucketBeginETA(newNode->ETA, curr);
                bucketNode *newbucket = CreateBucketNode(beginETA);
                newbucket->next = bptr;
                bprev->next = newbucket;
                newbucket->froot = InsertNode(newbucket->froot, newNode);
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
            newbucket->froot = InsertNode(newbucket->froot, newNode);
        }
    }
    return bhead;
}

/*--------------------------------------------------FILE HANDLING------------------------------------------------*/

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

        // if ETA > curr
        if (eta.totalmin >= curr.totalmin)
        {
            flightNode *newNode = CreateFlightNode(flightID, dt, eta);
            printf("\n");
            printFlight(newNode);
            bhead = InsertFlightInBucket(bhead, newNode, curr);
            hash[newNode->flightID][0] = newNode->ETA.totalmin;
            hash[newNode->flightID][1] = newNode->DT.totalmin;
        }
    }
    printf("\n");
    fclose(file);
    *bbhead = bhead;
}

/*----------------------------------------------------ADD NEW FLIGHT CHOICE-----------------------------------------*/
bucketNode *AddFlight(bucketNode *bhead, time curr)
{

    int flightID;
    printf("\nEnter flight ID:");
    scanf("%d", &flightID);
    time eta, dt;

    printf("\nEnter departure time: ");

    do
    {
        printf("\nHour: ");
        scanf("%d", &dt.hour);
    } while (dt.hour > 23 && printf("\nEnter valid hour."));
    do
    {
        printf("\nMin: ");
        scanf("%d", &dt.min);
    } while (dt.min > 59 && printf("\nEnter valid min."));
    dt = addAdjustTime(dt, 0);

    printf("\nEnter expected time of arrival: ");
    do
    {
        do
        {
            printf("\nHour: ");
            scanf("%d", &eta.hour);
        } while (eta.hour > 23 && printf("\nEnter valid hour."));
        do
        {
            printf("\nMin: ");
            scanf("%d", &eta.min);
        } while (eta.min > 59 && printf("\nEnter valid min."));
        eta = addAdjustTime(eta, 0);

    } while ((eta.totalmin < dt.totalmin || eta.totalmin < curr.totalmin) && printf("\nEnter valid time."));

    flightNode *newNode;
    newNode = CreateFlightNode(flightID, dt, eta);
    bhead = InsertFlightInBucket(bhead, newNode, curr);
    printf("\nAdded flight with id %d", flightID);
    // printf("\n-------------------------------Updated flight list -----------------------\n");
    // printAll(bhead);
    return bhead;
}

/*----------------------------------------------------SEARCH FUNCTIONS-------------------------------------------*/

int SearchFlightInBucket(flightNode *root, int fID, int dt_tt)
{
    flightNode *curr = root, *subcurr = NULL;
    int found = 0;
    while (curr != NULL && !found)
    {
        if (dt_tt == curr->DT.totalmin)
        {
            if (fID == curr->flightID)
            {
                found = 1;
                printf("\n");
                printFlight(curr);
            }
            else
            {
                subcurr = curr->subroot;
                while (subcurr != NULL && !found)
                {
                    if (fID == subcurr->flightID)
                    {
                        found = 1;
                        printf("\n");
                        printFlight(subcurr);
                    }
                    else if (fID < subcurr->flightID)
                    {
                        subcurr = subcurr->left;
                    }
                    else
                    {
                        subcurr = subcurr->right;
                    }
                }
            }
        }
        else if (dt_tt < curr->DT.totalmin)
        {
            curr = curr->left;
        }
        else
        {
            curr = curr->right;
        }
    }
    return found;
}

void ShowStatus(bucketNode *bhead, int fID)
{
    // Look for the right bucket;
    bucketNode *bptr = bhead;
    int found;
    if (bptr == NULL)
    {
        printf("\nNo data available");
    }
    else
    {
        int eta_tt = hash[fID][0];
        int dt_tt = hash[fID][1];
        while (bptr != NULL && eta_tt > bptr->endETA.totalmin)
        {
            bptr = bptr->next;
        }

        if (bptr == NULL)
        {
            printf("\nFlight not found");
        }
        else
        {
            if (bptr->beginETA.totalmin <= eta_tt && eta_tt <= bptr->endETA.totalmin)
            {
                found = SearchFlightInBucket(bptr->froot, fID, dt_tt);
                if (found)
                {
                    printf("\nFlight found.");
                }
                else
                {
                    printf("\nFlight not found");
                }
            }
            else
            {
                printf("\nFlight not found");
            }
        }
    }
}

/*----------------------------------------------------RANGE SEARCH-------------------------------------------*/
void rangesearch_flight(flightNode *root, int i_tt, int l_tt)
{
    if (root != NULL)
    {
        rangesearch_flight(root->left, i_tt, l_tt);
        if (root->ETA.totalmin >= i_tt && root->ETA.totalmin <= l_tt)
        {
            printFlight(root);
        }
        rangesearch_flight(root->subroot, i_tt, l_tt);
        rangesearch_flight(root->right, i_tt, l_tt);
    }
}

void RangeSearch(bucketNode *bhead, time initial, time last)
{
    bucketNode *bptr = bhead;
    int i_tt, l_tt;
    i_tt = initial.totalmin;
    l_tt = last.totalmin;

    if (bptr == NULL)
    {
        printf("Error, no data available\n");
    }
    while (bptr != NULL && i_tt > bptr->endETA.totalmin)
    {
        bptr = bptr->next;
    }
    while (bptr != NULL && l_tt >= bptr->beginETA.totalmin)
    {
        rangesearch_flight(bptr->froot, i_tt, l_tt);
        bptr = bptr->next;
    }
}

/*-----------------------------------------------WRITE TO FILE-----------------------------------------------*/
void writeFlightsToFile(const char *filename, bucketNode *bhead)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        fprintf(stderr, "Unable to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    bucketNode *bptr;
    bptr = bhead;
    if (bhead == NULL)
    {
        printf("Error, No bucket in the list\n");
    }
    while (bptr != NULL)
    {
        //fprintf(file, "%d,%d:%d,%d:%d\n", bptr->bucketID, bptr->beginETA.hour, bptr->beginETA.min, bptr->endETA.hour, bptr->endETA.min);
        printInOrderWrite(bptr->froot, file);
        printf("\n");
        bptr = bptr->next;
    }

    fclose(file);
}

/*------------------------------------------------MAIN---------------------------------------------------------*/

int main()
{
    bucketNode *bucketHead = NULL;
    char filename[20] = "FlightData.csv";
    int n, id;
    time currTime;

    printf("\nEnter current time: ");
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
        printf("\nPress 4: Search for flights between time interval.");
        printf("\nPress 5: Add flight.\n");
        printf("\nPress 6: Exit Program.\n");
        scanf("\n\n%d", &n);
        printf("\033[0;37m");
        if (n == 1) // Display all flights
        {
            printf("\n\033[0;33m------------------------------Today's Flights------------------------------\033[0m\n\n");
            printAll(bucketHead);
        }
        else if (n == 2) // Display a particular flight
        {
            printf("\n\nEnter the ID of the flight : ");
            scanf("%d", &id);
            // need to search then print
            ShowStatus(bucketHead, id);
        }
        else if (n == 3) // Cancel a particular flight
        {
            printf("\nEnter the ID of the flight to be cancelled: ");
            scanf("%d", &id);
            // need to search then delete
            bucketHead = CancelFlightPlan(bucketHead, id);
        }
        else if (n == 4) // Search for flights between time interval
        {
            time initial_time, last_time;
            printf("\nTo search, enter start time: ");
            do
            {
                do
                {
                    printf("\nHour: ");
                    scanf("%d", &initial_time.hour);
                } while (initial_time.hour > 23 && printf("\nEnter valid hour."));
                do
                {
                    printf("\nMin: ");
                    scanf("%d", &initial_time.min);
                } while (initial_time.min > 59 && printf("\nEnter valid min."));
                initial_time = addAdjustTime(initial_time, 0);

            } while (initial_time.totalmin < currTime.totalmin && printf("\nEnter valid time."));

            printf("\nTo search, enter end time: ");
            do
            {
                do
                {
                    printf("\nHour: ");
                    scanf("%d", &last_time.hour);
                } while (last_time.hour > 23 && last_time.totalmin < currTime.totalmin && printf("\nEnter valid hour."));
                do
                {
                    printf("\nMin: ");
                    scanf("%d", &last_time.min);
                } while (last_time.min > 59 && printf("\nEnter valid min."));
                last_time = addAdjustTime(last_time, 0);
            } while (last_time.totalmin < initial_time.totalmin && printf("\nEnter time after the start time."));
            printf("\n\nThe flights between time ");
            printTime(initial_time);
            printf(" and ");
            printTime(last_time);
            printf(" are: \n\n");
            RangeSearch(bucketHead, initial_time, last_time);
        }
        else if (n == 5) // Add flight
        {
            AddFlight(bucketHead, currTime);
            printf("\n-------------------------------Updated flight list -----------------------\n");
            printAll(bucketHead);
        }
        else if (n > 6 || n < 1)
        {
            printf("\nEnter valid number.");
        }

    } while (n != 6);
    printf("\n\033[0;33m------------------------------Thank you!------------------------------\033[0m\n\n");
    const char *outputFilename = "updated_flights.txt";
    writeFlightsToFile(outputFilename, bucketHead);
    printf("Flights have been written to %s\n", outputFilename);
    return 0;
}