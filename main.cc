/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

/* FUNCTION DECLARATIONS */

void *producer (void *id);
void *consumer (void *id);

/* STRUCT DECLARATION */

struct Job {
  int jobId;
  int duration;
};

struct ProducerInfo {
  int producerid;
  int &rear;
  int &numberOfJobs;
  int queueSize;
  Job* queuePtr;
  int semId;
};

struct ConsumerInfo {
  int consumerid;
  int &front;
  int &totalNumberOfJobsToConsume;
  int queueSize;
  Job* queuePtr;
  int semId;
};

int main (int argc, char **argv)
{
  // Initialise argument line variables
  int queueSize, numberOfJobs, numberOfProducers, numberOfConsumers, totalNumberOfJobsToConsume;
  
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
    totalNumberOfJobsToConsume = numberOfJobs * numberOfProducers;
  }
  cout << "Queue Size: " << queueSize << ", number of jobs: " << numberOfJobs << ", number of producers: " << numberOfProducers << ", number of consumers: " << numberOfConsumers << endl;
  
  // Create queue data structure
  Job* queue = new Job[queueSize];
  int rear=0, front=0;

  // Creating semaphore array of 3 semaphores
  int semId = sem_create(SEM_KEY, 3);
  if (semId < 0) cerr << "Error creating semaphore array\n";

  /* Initialise Semaphores */
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
    ProducerInfo producerInfo = {.producerid = (iterator+1), .rear = rear, 
                                  .numberOfJobs = numberOfJobs, .queueSize = queueSize, 
                                  .queuePtr = queue, .semId = semId};

    pthread_create (&thrProducers[iterator], NULL, producer, (void *) &producerInfo);
  }

  // Create a thread for each consumer
  pthread_t thrConsumers[numberOfConsumers];
  for (auto iterator = 0; iterator < numberOfConsumers; iterator++) {
    // Parse in consumer info
    ConsumerInfo consumerInfo = {.consumerid = (iterator+1), .front = front, 
                                  .totalNumberOfJobsToConsume = totalNumberOfJobsToConsume, .queueSize = queueSize, 
                                  .queuePtr = queue, .semId = semId};
    cout << "Consumer info front: " << consumerInfo.front << endl;

    pthread_create (&thrConsumers[iterator], NULL, consumer, (void *) &consumerInfo);
  }

  // Block until all threads complete
  for (auto iterator = 0; iterator < numberOfProducers; iterator++) {
    if (pthread_join(thrProducers[iterator], NULL) != 0) cerr << "Error joining producer threads\n";
    else cout << "Doing some producer work after the join!\n";
  }
  for (auto iterator = 0; iterator < numberOfConsumers; iterator++) {
    if (pthread_join(thrConsumers[iterator], NULL) != 0) cerr << "Error joining consumer threads\n";
    else cout << "Doing some consumer work after the join!\n";
  }
  
  // Clean up semaphores
  if (sem_close(semId) < 0) cerr << "Error closing off semaphore array\n";

  return 0;
}

void *producer (void *producerInfo) 
{
  ProducerInfo *info = (ProducerInfo*) producerInfo;
  cout << "producer info: number of jobs: " << info->numberOfJobs << endl;

  while (info->numberOfJobs > 0) {
    cout << "Number of jobs for current producer: " << info->numberOfJobs << endl;
    // check for space
    sem_wait(info->semId, 1);
    // LOCK
    sem_wait(info->semId, 0);

    /* START OF CRITICAL SECTION */

    // Add job to buffer
    info->queuePtr[info->rear].jobId = info->rear;
    info->queuePtr[info->rear].duration = rand() % 10 + 1;
    cout << "Producer(" << info->producerid << "): Job id " << info->rear 
          << " duration " << info->queuePtr[info->rear].duration << endl;

    // increment end of queue and implement circular queue
    info->rear = (info->rear + 1)%(info->queueSize);

    /* END OF CRITICAL SECTION */

    // UNLOCK
    sem_signal(info->semId, 0);
    // let consumer know of job item
    sem_signal(info->semId, 2);

    // sleep for 1-5 seconds before next job can be added
    int addJobInterval = rand() % 5 + 1;
    cout << "Adding job interval; sleep for duration of: " << addJobInterval << endl;
    sleep(1); // THIS NEEDS TO BE CHANGED

    info->numberOfJobs --;
  }

  pthread_exit(0);
}

void *consumer (void *consumerInfo) 
{
  ConsumerInfo *info = (ConsumerInfo*) consumerInfo;

  // cout << "Consumer info front: " << info->front << endl;

  while(info->totalNumberOfJobsToConsume > 0) {
    cout << "Total number of jobs left to consume: " << info->totalNumberOfJobsToConsume << endl;
    int sleepDuration;

    // Check for job item
    sem_wait(info->semId, 2);
    // LOCK
    sem_wait(info->semId, 0);

    /* START OF CRITICAL SECTION */

    // get job from buffer
    cout << "Consumer(" << info->consumerid << "): Job id " << info->front 
          << " executing sleep duration " << info->queuePtr[info->front].duration << endl;
    
    // save sleep duration
    sleepDuration = info->queuePtr[info->front].duration;

    // increment beginning of queue and implement circular queue
    info->front = (info->front + 1)%(info->queueSize);

    /* END OF CRITICAL SECTION */

    // UNLOCK
    sem_signal(info->semId, 0);
    // let producer know of space
    sem_signal(info->semId, 1);

    // consume job item
    cout << "Time to sleep for: " << sleepDuration << " seconds\n";
    sleep(1); // THIS NEEDS TO BE CHANGED

    info->totalNumberOfJobsToConsume--;
  }

  pthread_exit (0);
}