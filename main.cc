/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include <typeinfo>
#include "helper.h"

void *producer (void *id);
void *consumer (void *id);

struct Job {
  int id = -1;  // Job has no ID yet
  int duration = rand() % 10 + 1;
};

struct ProducerInfo {
  int producerid;
  int rear;
  int numberOfJobs;
  int queueSize;
  Job* queuePtr;
};

struct ConsumerInfo {
  int consumerid;
  int front;
  int queueSize;
  Job* queuePtr;
};

int main (int argc, char **argv)
{

  // TODO

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
  cout << "Queue Size: " << queueSize << ", number of jobs: " << numberOfJobs << ", number of producers: " << numberOfProducers << ", number of consumers: " << numberOfConsumers << endl;
  
  // Create queue data structure
  Job* queue = new Job[queueSize];

  // Initialise rear and front of queue
  int rear=0, front=0;

  // // Creating semaphore array of 3 semaphores
  // int semId = sem_create(SEM_KEY, 3);

  // /* Initialise Semaphores */
  // // First semaphore is for Mutual Exclusivity
  // int mutex = sem_init(semId, 0, 1);
  // // Second semaphore is to check for space in buffer
  // int space = sem_init(semId, 1, queueSize);
  // // Third semaphore is for consumer item
  // int item = sem_init(semId, 2, 0);

  // cout << mutex << endl << space << endl << item << endl;

  for (auto iterator = 0; iterator < numberOfProducers; ++iterator) {
    // Create a thread for each producer
    pthread_t producerid;
    ProducerInfo producerInfo = {.producerid = (iterator+1), .rear = rear, .numberOfJobs = numberOfJobs, .queueSize = queueSize, .queuePtr = queue};
    
    pthread_create (&producerid, NULL, producer, (void *) &producerInfo);

    pthread_join (producerid, NULL);
    cout << "Doing some producer work after the join" << endl;
  }

  for (auto iterator = 0; iterator < numberOfConsumers; ++iterator) {
    // Create a thread for each consumer
    pthread_t consumerid;
    ConsumerInfo consumerInfo = {.consumerid = (iterator+1), .front = front, .queueSize = queueSize, .queuePtr = queue};

    pthread_create (&consumerid, NULL, consumer, (void *) &consumerInfo);

    pthread_join (consumerid, NULL);
    cout << "Doing some consumer work after the join" << endl;
  }

  return 0;
}

void *producer (void *producerInfo) 
{
  ProducerInfo *info = (ProducerInfo*) producerInfo;

  while (info->numberOfJobs > 0) {
    if (info->queuePtr[info->rear].id < 0) {
      cout << "No job present! Current rear: " << info->rear << endl;
      cout << "Producer(" << info->producerid << "): Job id " << info->rear 
            << " duration " << info->queuePtr[info->rear].duration << endl;

      // add a job and increment end of queue
      info->queuePtr[info->rear].id = info->rear;
      info->rear++;
      if (info->rear == info->queueSize) info->rear = 0;  // implement circular queue


      cout << "Job added! New rear: " << info->rear << endl;

      // sleep for 1-5 seconds before next job can be added
      int addJobInterval = rand() % 5 + 1;
      cout << "Adding job interval; sleep for duration of: " << addJobInterval << endl;
      sleep(addJobInterval);
    }

    // Use semaphores to ensure concurrency
    // sem_wait(info->rear, 1);

    info->numberOfJobs --;

  }
  pthread_exit(0);

}

void *consumer (void *consumerInfo) 
{
  ConsumerInfo *info = (ConsumerInfo*) consumerInfo;
  cout << "Current job id: " << info->queuePtr[info->front].id << endl;
  while (info->queuePtr[info->front].id >= 0) {
    cout << "Job present! Current front: " << info->front << endl;
    cout << "Consumer(" << info->consumerid << "): Job id " << info->front 
          << " executing sleep duration " << info->queuePtr[info->front].duration << endl;
    
    // save sleep duration
    int sleepDuration = info->queuePtr[info->front].duration;

    // remove the job and increment beginning of queue
    info->queuePtr[info->front].id = -1;
    info->front++;
    if (info->front == info->queueSize) info->front = 0;  // implement circular queue

    cout << "Job removed! New front: " << info->front << endl;

    cout << "Time to sleep for: " << sleepDuration << " seconds\n";

    // sleep(sleepDuration);
    cout << "\nThat was a good consumer sleep - thank you \n" << endl;
  }

  pthread_exit (0);

}