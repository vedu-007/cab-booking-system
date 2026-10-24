/* Cab Booking System - graph + Dijkstra + driver array + linked list */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_LOCATIONS 6
#define NUM_DRIVERS   4
#define NAME_LENGTH   30
#define INF           1000000      /* no route */
#define NONE          -1
#define BASE_FARE     30
#define RATE_PER_KM   12
struct Road {
    int to, km;
    struct Road *next;
};
struct Driver {
    int  id;
    char name[NAME_LENGTH + 1];
    int  location, available;
};
struct Ride {
    int  id;
    char rider[NAME_LENGTH + 1];
    int  driverId, pickup, destination, fare;
    int  completed;
    struct Ride *next;
};

const char *placeName[NUM_LOCATIONS] = {
    "Andheri", "Bandra", "Dadar", "Powai", "Kurla", "Ghatkopar"
};
struct Road *cityMap[NUM_LOCATIONS];
struct Driver drivers[NUM_DRIVERS] = {
    {101, "Ravi",  0, 1},
    {102, "Sunil", 2, 1},
    {103, "Meena", 3, 1},
    {104, "Imran", 5, 1}
};
struct Ride *history = NULL;
int nextRideId = 1;

void readLine(char *buffer, int size) {
    int ch;
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
    } else if (strchr(buffer, '\n') != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
    } else {
        while ((ch = getchar()) != '\n' && ch != EOF) { }
    }
}

int readNumber(void) {
    char line[20], *end;
    long value;
    readLine(line, sizeof(line));
    if (line[0] == '\0') return NONE;
    value = strtol(line, &end, 10);
    if (*end != '\0' || value < 0 || value > 100000) return NONE;
    return (int)value;
}

int readLocation(const char *prompt) {
    int choice;
    printf("%s (1-%d): ", prompt, NUM_LOCATIONS);
    choice = readNumber();
    if (choice < 1 || choice > NUM_LOCATIONS) { printf("Invalid location number.\n"); return NONE; }
    return choice - 1;
}

void addRoad(int a, int b, int km) {
    int side, from, to;
    struct Road *road;
    for (side = 0; side < 2; side++) {
        from = (side == 0) ? a : b;
        to   = (side == 0) ? b : a;
        road = (struct Road *)malloc(sizeof(struct Road));
        if (road == NULL) { printf("Out of memory.\n"); exit(1); }
        road->to = to;
        road->km = km;
        road->next = cityMap[from];
        cityMap[from] = road;
    }
}
void buildCityMap(void) {

    int roads[8][3] = {
        {0, 1, 8}, {0, 3, 7}, {1, 2, 7}, {1, 4, 6},
        {2, 4, 6}, {3, 4, 8}, {3, 5, 6}, {4, 5, 4}
    };
    int i;
    for (i = 0; i < 8; i++) addRoad(roads[i][0], roads[i][1], roads[i][2]);
}

/* shortest distance from source to every location */
void dijkstra(int source, int dist[], int prev[]) {
    int visited[NUM_LOCATIONS] = {0};
    int i, round, current;
    struct Road *road;

    for (i = 0; i < NUM_LOCATIONS; i++) { dist[i] = INF; prev[i] = NONE; }
    dist[source] = 0;
    for (round = 0; round < NUM_LOCATIONS; round++) {

        current = NONE;
        for (i = 0; i < NUM_LOCATIONS; i++)
            if (!visited[i] && dist[i] < INF && (current == NONE || dist[i] < dist[current]))
                current = i;
        if (current == NONE) break;
        visited[current] = 1;

        for (road = cityMap[current]; road != NULL; road = road->next)
            if (dist[current] + road->km < dist[road->to]) {
                dist[road->to] = dist[current] + road->km;
                prev[road->to] = current;
            }
    }
}

void printRoute(int place, int prev[]) {
    if (prev[place] != NONE) { printRoute(prev[place], prev); printf(" -> "); }
    printf("%s", placeName[place]);
}

int findDriverById(int id) {
    int i;
    for (i = 0; i < NUM_DRIVERS; i++)
        if (drivers[i].id == id) return i;
    return NONE;
}

/* nearest available driver; smaller ID wins a tie */
int findNearestDriver(int dist[]) {
    int i, best = NONE;
    for (i = 0; i < NUM_DRIVERS; i++) {
        int d = dist[drivers[i].location];
        if (!drivers[i].available || d == INF) continue;
        if (best == NONE || d < dist[drivers[best].location] ||
            (d == dist[drivers[best].location] && drivers[i].id < drivers[best].id))
            best = i;
    }
    return best;
}

void viewLocations(void) {
    int i;
    struct Road *road;
    printf("\n--- Locations and roads (sample distances) ---\n");
    for (i = 0; i < NUM_LOCATIONS; i++) {
        printf("%d. %-10s: ", i + 1, placeName[i]);
        for (road = cityMap[i]; road != NULL; road = road->next)
            printf("%s (%d km)  ", placeName[road->to], road->km);
        printf("\n");
    }
}

void viewDrivers(void) {
    int i;
    printf("\n--- Drivers ---\n%-5s %-8s %-12s %s\n", "ID", "Name", "Location", "Status");
    for (i = 0; i < NUM_DRIVERS; i++)
        printf("%-5d %-8s %-12s %s\n", drivers[i].id, drivers[i].name,
               placeName[drivers[i].location],
               drivers[i].available ? "Available" : "Busy");
}

void bookRide(void) {
    char rider[NAME_LENGTH + 1];
    int pickup, destination, d, tripKm, fare, i;
    int dist[NUM_LOCATIONS], prev[NUM_LOCATIONS];
    struct Ride *ride;
    printf("\n--- Book a Ride ---\nEnter rider name (max %d characters): ", NAME_LENGTH);
    readLine(rider, sizeof(rider));
    if (rider[0] == '\0') { printf("Name cannot be empty.\n"); return; }
    for (i = 0; i < NUM_LOCATIONS; i++) printf("%d. %s\n", i + 1, placeName[i]);
    pickup = readLocation("Enter pickup location");
    if (pickup == NONE) return;
    destination = readLocation("Enter destination location");
    if (destination == NONE) return;
    if (pickup == destination) { printf("Pickup and destination cannot be the same.\n"); return; }

    dijkstra(pickup, dist, prev);
    if (dist[destination] == INF) {
        printf("Sorry, no route found from %s to %s.\n", placeName[pickup], placeName[destination]);
        return;
    }
    d = findNearestDriver(dist);
    if (d == NONE) { printf("Sorry, no available driver can reach %s.\n", placeName[pickup]); return; }

    tripKm = dist[destination];
    fare = BASE_FARE + RATE_PER_KM * tripKm;
    ride = (struct Ride *)malloc(sizeof(struct Ride));
    if (ride == NULL) { printf("Out of memory.\n"); exit(1); }
    ride->id = nextRideId++;
    strcpy(ride->rider, rider);
    ride->driverId = drivers[d].id;
    ride->pickup = pickup;
    ride->destination = destination;
    ride->fare = fare;
    ride->completed = 0;
    ride->next = history;                  /* add at the front */
    history = ride;
    drivers[d].available = 0;
    printf("\nBooking confirmed!\n");
    printf("Ride ID            : %d\nRider              : %s\n", ride->id, ride->rider);
    printf("Driver             : %s (ID %d)\n", drivers[d].name, drivers[d].id);
    printf("Distance to pickup : %d km (driver is at %s)\n",
           dist[drivers[d].location], placeName[drivers[d].location]);
    printf("Trip route         : ");
    printRoute(destination, prev);
    printf("\nTrip distance      : %d km\n", tripKm);
    printf("Total fare         : Rs. %d (Rs. %d base + Rs. %d x %d km)\n",
           fare, BASE_FARE, RATE_PER_KM, tripKm);
}

void completeRide(void) {
    int id, d;
    struct Ride *ride;
    printf("\n--- Complete a Ride ---\nEnter ride ID: ");
    id = readNumber();
    for (ride = history; ride != NULL && ride->id != id; ride = ride->next) { }
    if (ride == NULL) { printf("No ride found with that ID.\n"); return; }
    if (ride->completed) { printf("Ride %d is already completed.\n", id); return; }
    ride->completed = 1;
    d = findDriverById(ride->driverId);
    drivers[d].location = ride->destination;
    drivers[d].available = 1;
    printf("Ride %d completed. %s is now available at %s.\n",
           id, drivers[d].name, placeName[ride->destination]);
}

void viewHistory(void) {
    struct Ride *ride;
    printf("\n--- Ride History (latest first) ---\n");
    if (history == NULL) { printf("No rides booked yet.\n"); return; }
    printf("%-4s %-15s %-8s %-10s %-10s %-9s %s\n",
           "ID", "Rider", "Driver", "Pickup", "Drop", "Fare", "Status");
    for (ride = history; ride != NULL; ride = ride->next)
        printf("%-4d %-15s %-8s %-10s %-10s Rs.%-6d %s\n", ride->id, ride->rider,
               drivers[findDriverById(ride->driverId)].name,
               placeName[ride->pickup], placeName[ride->destination], ride->fare,
               ride->completed ? "Completed" : "Active");
}

void freeEverything(void) {
    int i;
    struct Road *road;
    struct Ride *ride;
    for (i = 0; i < NUM_LOCATIONS; i++)
        while (cityMap[i] != NULL) {
            road = cityMap[i];
            cityMap[i] = road->next;
            free(road);
        }
    while (history != NULL) {
        ride = history;
        history = ride->next;
        free(ride);
    }
}

int main(void) {
    int choice;
    buildCityMap();
    printf("=== Simple Ride Booking System ===\n(Distances are approximate sample values.)\n");
    do {
        printf("\n===== MENU =====\n1. View locations\n2. View drivers\n3. Book a ride\n");
        printf("4. Complete a ride\n5. View ride history\n6. Exit\nEnter your choice: ");
        choice = readNumber();
        switch (choice) {
            case 1: viewLocations(); break;
            case 2: viewDrivers();   break;
            case 3: bookRide();      break;
            case 4: completeRide();  break;
            case 5: viewHistory();   break;
            case 6: printf("Goodbye!\n"); break;
            default: printf("Invalid choice. Enter a number from 1 to 6.\n");
        }
    } while (choice != 6);
    freeEverything();
    return 0;
}
