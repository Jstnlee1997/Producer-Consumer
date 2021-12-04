/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

/* GLOBAL CONSTANTS */
# define CONSUMER_TIMEOUT 20

/* FUNCTION DECLARATIONS */

void *producer (void *id);
void *consumer (void *id);

/* STRUCT DECLARATIONS */

struct Job {
  int jobId;
  int duration;
};

struct ProducerInfo {
  int producerid;
  int numberOfJobs;
  int queueSize;
  Job* queuePtr;

  friend ostream& operator<<(ostream &o, const ProducerInfo p) {
    return o << "Producer id: " << p.producerid << " , number of jobs: "<< p.numberOfJobs 
              << ", queue size: " << p.queueSize << endl;
  };
};

struct ConsumerInfo {
  int consumerid;
  int queueSize;
  Job* queuePtr;
};

struct timespec timeout = {CONSUMER_TIMEOUT};

/* GLOBAL VARIABLES */
int rear = 0;
int front = 0;

int semId = sem_create(SEM_KEY, 3);

int main (int argc, char **argv)
{
  // Initialise argument line variables
  int queueSize, numberOfJobs, numberOfProducers, numberOfConsumers;
  
  // Ensure that there are four argument line arguments (5 including execution)
  if (argc != 5) {
    cerr << "There has to be 4 argument line arguments!\n"; 
    return 0;
  }

  // Receive input of command line arguments and ensure that they are appropriate
  if (check_arg(argv[1]) == -1 || check_arg(argv[2]) == -1 || check_arg(argv[3]) == -1 || check_arg(argv[4]) == -1) {
    cerr << "Argument line arguments have to be integers!\n";
    return 0;
  } else {
    queueSize = check_arg(argv[1]);
    numberOfJobs = check_arg(argv[2]);
    numberOfProducers = check_arg(argv[3]);
    numberOfConsumers = check_arg(argv[4]);
  }
  cout << "Queue Size = " << queueSize << ", Number of Jobs produced per producer = " << numberOfJobs 
        << ", Number of producers = " << numberOfProducers << ", and Number of consumers = " << numberOfConsumers << endl;
  
  // Create queue data structure
  Job* queue = new Job[queueSize];

  /* Initialise Semaphores */
  // Check if sempahore array initialised expectedly
  if (semId < 0) cerr << "Error creating semaphore array\n";

  // First semaphore is for Mutual Exclusivity
  int mutex = sem_init(semId, 0, 1);
  // Second semaphore is to check for space in buffer
  int space = sem_init(semId, 1, queueSize);
  // Third semaphore is for consumer item
  int item = sem_init(semId, 2, 0);
  if (mutex < 0 || space < 0 || item < 0) cerr << "Error initialising semaphores\n";

  // Create a thread for each producer
  pthread_t thrProducers[numberOfProducers];
  for (auto iterator = 0; iterator < numberOfProducers; iterator++) {
    // Parse in producer info
    ProducerInfo* producerInfo = new ProducerInfo {(iterator+1), numberOfJobs, queueSize, queue};

    pthread_create (&thrProducers[iterator], NULL, producer, (void *) producerInfo);
  }

  // Create a thread for each consumer
  pthread_t thrConsumers[numberOfConsumers];
  for (auto iterator = 0; iterator < numberOfConsumers; iterator++) {
    // Parse in consumer info
    ConsumerInfo* consumerInfo = new ConsumerInfo {(iterator+1), queueSize, queue};

    pthread_create (&thrConsumers[iterator], NULL, consumer, (void *) consumerInfo);
  }

  // Block until all threads complete
  for (auto iterator = 0; iterator < numberOfProducers; iterator++) {
    if (pthread_join(thrProducers[iterator], NULL) != 0) cerr << "Error joining producer threads\n";
  }
  for (auto iterator = 0; iterator < numberOfConsumers; iterator++) {
    if (pthread_join(thrConsumers[iterator], NULL) != 0) cerr << "Error joining consumer threads\n";
  }
  
  // Clean up semaphores
  if (sem_close(semId) < 0) cerr << "Error closing off semaphore array\n";

  return 0;
}

void *producer (void *producerInfo) 
{
  ProducerInfo *info = (ProducerInfo*) producerInfo;

  while (info->numberOfJobs > 0) {
    // check for space
    sem_wait(semId, 1);
    // LOCK
    sem_wait(semId, 0);

    /* START OF CRITICAL SECTION */

    // Add job to buffer
    info->queuePtr[rear].jobId = rear;
    info->queuePtr[rear].duration = rand() % 10 + 1;
    cout << "Producer(" << info->producerid << "): Job id " << rear 
          << " duration " << info->queuePtr[rear].duration << endl;

    // increment end of queue and implement circular queue
    rear ++;
    if (rear == info->queueSize) rear = 0;

    /* END OF CRITICAL SECTION */

    // UNLOCK
    sem_signal(semId, 0);
    // let consumer know of job item
    sem_signal(semId, 2);

    info->numberOfJobs --;
    if (info->numberOfJobs == 0) {
      cout << "Producer(" << info->producerid << "): No more jobs to generate.\n";
    } else {
      // sleep for 1-5 seconds before next job can be added
      int addJobInterval = rand() % 5 + 1;
      sleep(addJobInterval);
    }
  }

  pthread_exit(0);
}

void *consumer (void *consumerInfo) 
{
  ConsumerInfo *info = (ConsumerInfo*) consumerInfo;

  while(1) {
    int jobId;
    int sleepDuration;

    // Check for job item
    if (sem_wait(semId, 2, &timeout) < 0) {
      cout << "Consumer(" << info->consumerid << "): No more jobs left.\n";
      break;
    } 

    // LOCK
    sem_wait(semId, 0);

    /* START OF CRITICAL SECTION */

    // get job from buffer
    jobId = front;
    cout << "Consumer(" << info->consumerid << "): Job id " << jobId 
          << " executing sleep duration " << info->queuePtr[front].duration << endl;
    
    // save sleep duration
    sleepDuration = info->queuePtr[front].duration;

    // increment beginning of queue and implement circular queue
    front ++;
    if (front == info->queueSize) front = 0;

    /* END OF CRITICAL SECTION */

    // UNLOCK
    sem_signal(semId, 0);
    // let producer know of space
    sem_signal(semId, 1);

    // consume job item
    sleep(sleepDuration);
    cout << "Consumer(" << info->consumerid << "): Job id " << jobId 
          << " completed" << endl;
  }

  pthread_exit (0);
}