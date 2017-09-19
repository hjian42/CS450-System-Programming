/* THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS 
- Hang Jiang */

// scp goldbach.c hjian42@lab0z.mathcs.emory.edu:/home/hjian42/cs450/project1

#include <stdio.h>
#include <stdlib.h>

typedef struct _seg{
    int bits[256];
    struct _seg *prev, *next;
} seg;

// define head pointer
seg *head = NULL;
// seg *lastSeg = NULL;

typedef struct _pointer{
	int indexOfBit, indexOfArray;
	seg* segment;
} pointer;

// define left and right pointers
pointer* leftPoint;
pointer* rightPoint;

// define methods 
seg* initialize(int);
seg* whichseg(int);
int whichint(int);
int whichbit(int);
int marknonprime(int);
int testprime(int);
int countPrimes(int);
void findPairs(int,int);
void sieveOfE(int);

# define segmentByteNumber (8*256*sizeof(int)) // one int: 4 bytes OR 32 bits
# define seg_struct_size sizeof(seg) // # of bytes in seg  
# define intSize 32 // # of bits in int
# define segIntNumber 256// # of ints in bits of seg 
# define getBitPosition(N) (N-2)/2 // get the bit position of an odd prime

int main(int argc, char *argv[]) {

	int N, numOfPrimes;
	if (argc == 2){
		sscanf(argv[1],"%d",&N);
	} else {
		printf("Enter a number = ");
		scanf("%d",&N);
	}

	N = atoi(argv[1]);

	int numberOfSegs = ((N-3) / 2 / segmentByteNumber) + 1;

	// create numerOfSegs of segs to store N bits 
	head = initialize(numberOfSegs);
	// printf("Done allocating %d nodes\n",numberOfSegs);

	// seiveOfE algorithm
	sieveOfE(N); // build the doubled linked list and performs sieveOfE algorithm

	numOfPrimes = countPrimes(N); // count the # of primes
	printf("Calculating odd primes up to %d...\n", N);
	printf("The number of odd primes less than or equal to %d is %d \n", N, numOfPrimes);

	int N2;
	printf("Enter Even Numbers >5 for Goldbach Tests: \n");
	while ((scanf("%d", &N2)) != EOF){
		// printf("The input is %d\n", N2);
		if (N2 < 5) {
			continue;
		} else if ( N2%2 == 1 ) {
			continue;
		} else if ( N2 > N ){
			continue;
		}
		findPairs(N2,N);
	}
}


seg* initialize(int numberOfSegs) {
	seg* curr_seg = (seg*)malloc(seg_struct_size);
	seg* prev_seg = NULL;
	seg* first = curr_seg;
	for (int k = 0; k < segIntNumber; k++) {
		curr_seg->bits[k]=0;
	}
	while (numberOfSegs > 1) {
		curr_seg->prev = prev_seg;
		seg* new_seg = (seg*)malloc(seg_struct_size);
		for (int k = 0; k < segIntNumber; k++) {
			new_seg->bits[k]=0;
		}
		curr_seg->next = new_seg;
		prev_seg = curr_seg;
		curr_seg = new_seg;
		numberOfSegs--;
	}
	curr_seg->prev = prev_seg;
	curr_seg->next = NULL;
	return first;
}

void findPairs(int N2, int N) {
	int count = 0;
	int upperBound = N2/2;
	int left = 3;
	int right = N2-3;
	int best_left = 3;
	int best_right = N;

	leftPoint = (pointer*)malloc(sizeof(pointer));
	rightPoint = (pointer*)malloc(sizeof(pointer));

	leftPoint->segment = whichseg(left);
	leftPoint->indexOfArray = whichint(left);
	leftPoint->indexOfBit = whichbit(left);
	rightPoint->segment = whichseg(right);
	rightPoint->indexOfArray = whichint(right);
	rightPoint->indexOfBit = whichbit(right);

	while (1) {

		if (rightPoint->indexOfBit < 0) {
			rightPoint->indexOfBit = intSize-1;
			rightPoint->indexOfArray --;
			if (rightPoint->indexOfArray < 0) {
				rightPoint->indexOfArray = segIntNumber-1;
				rightPoint->segment = rightPoint->segment->prev;
			}
		}

		if (leftPoint->indexOfBit > intSize-1) {
			leftPoint->indexOfBit = 0;
			leftPoint->indexOfArray ++;
			if (leftPoint->indexOfArray > segIntNumber-1) {
				leftPoint->indexOfArray = 0;
				leftPoint->segment = leftPoint->segment->next;
			}
		}

		if (!((leftPoint->segment->bits[leftPoint->indexOfArray])&(1<<leftPoint->indexOfBit))
			&& !((rightPoint->segment->bits[rightPoint->indexOfArray])&(1<<rightPoint->indexOfBit))) {
			count ++;
			best_left = left;
			best_right = right;
		}
		
		leftPoint->indexOfBit += 1;
		rightPoint->indexOfBit -= 1;
		left += 2;
		right -= 2;
		if (right < left) break;
	}
	printf("Largest %d =  %d + %d out of %d solutions\n", N2, best_left, best_right, count);

}


seg* whichseg(int i) {
	static int prev = 3;
	static int prev_seg_number = 0;
	static seg* lastSeg = NULL;
	int j=0;
	int seg_number = getBitPosition(i) / segmentByteNumber;
	seg* curr_seg;
	if (i > prev) {
		// int prev_seg_number = getBitPosition(prev) /segmentByteNumber;
		j = prev_seg_number;
		curr_seg = lastSeg;
	} else {
		curr_seg = head;
	}
	
	for (; j<seg_number; j++) {
		curr_seg = curr_seg->next;
	}
	// update the static variables 
	lastSeg = curr_seg;
	prev = i;
	prev_seg_number = seg_number;
	return curr_seg;
}

int whichint(int i) {
	return (getBitPosition(i) % segmentByteNumber) / intSize;
}

int whichbit(int i) {
	return getBitPosition(i) % intSize; // omit % segmentByteNumber because it is multiple of 32
}

int marknonprime(int i) {
	return whichseg(i)->bits[whichint(i)] |= (1<<whichbit(i)); // returned value not used in the program 
}

int testprime(int i) {
	return whichseg(i)->bits[(whichint(i))] & (1<<whichbit(i)); // 0 if 0 and others if 1 (not prime)
}

void sieveOfE(int N) {
	for (int i=3; i <= N; i+=2) {
		if (i*i> N) break;
		if (testprime(i) != 0) {
			continue;
		}
		for (int j=3*i; j<=N; j+=2*i) {
			marknonprime(j);
		}
	}
}

int countPrimes(int N) {
	int count = 0;
	for (int i=3; i <= N; i+=2) {
		if (testprime(i) != 0) {
			continue;
		}
		count ++;
	}
	return count;
}









