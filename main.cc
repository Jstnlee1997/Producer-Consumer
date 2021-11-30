/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

void *producer (void *id);
void *consumer (void *id);

struct Job {
  int id;
  int duration = rand() % 10 + 1;
};

struct ProducerInfo {
  int rear;
  int numberOfJobs;
  Job* queuePtr;
  friend ostream& operator<<(ostream& o, const ProducerInfo& producerInfo) {
    return o << "Current rear: " << producerInfo.rear << endl;
  }
};

struct ConsumerInfo {
  int front;
  Job* queuePtr;
  friend ostream& operator<<(ostream& o, const ConsumerInfo& consumerInfo) {
    return o << "Current front: " << consumerInfo.front << endl;
  }
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

  // Create queue data structure
  Job *queue = new Job[queueSize];
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
    ProducerInfo producerInfo = {.rear = rear, .numberOfJobs = numberOfJobs, .queuePtr = queue};
    
    pthread_create (&producerid, NULL, producer, (void *) &producerInfo);

    pthread_join (producerid, NULL);
    cout << "Doing some producer work after the join" << endl;
  }

  for (auto iterator = 0; iterator < numberOfConsumers; ++iterator) {
    // Create a thread for each consumer
    pthread_t consumerid;
    ConsumerInfo consumerInfo = {.front = front, .queuePtr = queue};

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

    cout << *info << endl;

    // Use semaphores to ensure concurrency
    // sem_wait(info->rear, 1);

    info->numberOfJobs --;

  }
  pthread_exit(0);

}

void *consumer (void *consumerInfo) 
{
    // TODO 

  sleep (1);

  cout << "\nThat was a good consumer sleep - thank you \n" << endl;

  pthread_exit (0);

}