#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

struct City {
    int x;
    int y;
};

struct Individual {
    int* path;
    double fitness;
};

struct City* cities;
struct Individual* population;

int numCities = 50; //���� ��
int numGenerations = 1000; //��ȭ �ݺ� ��
int numGenes = 50; //������ �� 

// ���Ͽ��� ������ ��ǥ�� �а� ��ü �迭�� ����
void readCoordinates(const char* filename) {
    FILE* file;
    if ((file = fopen(filename, "r")) == NULL) {
        printf("Failed to open file.\n");
        exit(1);
    }

    cities = (struct City*)malloc(numCities * sizeof(struct City));

    for (int i = 0; i < numCities; i++) {
        int result = fscanf(file, "%d %d", &(cities[i].x), &(cities[i].y));
        if (result != 2) {
            printf("Error reading coordinates from file.\n");
            fclose(file);
            free(cities);
            exit(1);
        }
    }

    fclose(file);
}

// �� ���� ���� �Ÿ��� ����ϱ� ���� �Ÿ� ���� ����Ͽ� �Ÿ� ���
double calculateDistance(struct City city1, struct City city2) {
    int dx = city1.x - city2.x;
    int dy = city1.y - city2.y;
    return sqrt(dx * dx + dy * dy);
}

// ���յ� ���: �̵��� ��ü �Ÿ��� ������� �ϸ�, �Ÿ��� ª������ ���� ���յ� ��
double calculateFitness(struct Individual* individual) {
    double totalDistance = 0.0;

    for (int i = 0; i < numGenes - 1; i++) {
        int cityIndex1 = individual->path[i];
        int cityIndex2 = individual->path[i + 1];
        struct City city1 = cities[cityIndex1];
        struct City city2 = cities[cityIndex2];
        totalDistance += calculateDistance(city1, city2);
    }

    int lastCityIndex = individual->path[numGenes - 1];
    struct City lastCity = cities[lastCityIndex];
    struct City startingCity = cities[individual->path[0]];
    totalDistance += calculateDistance(lastCity, startingCity);

    return 1.0 / totalDistance;
}

// �������� �������� �ʱ�ȭ
void initializePopulation() {
    population = (struct Individual*)malloc(numCities * sizeof(struct Individual));

    for (int i = 0; i < numCities; i++) {
        population[i].path = (int*)malloc(numGenes * sizeof(int));
        for (int j = 0; j < numGenes; j++) {
            population[i].path[j] = j;
        }
    }

    for (int i = 0; i < numCities; i++) {
        for (int j = 0; j < numGenes; j++) {
            int randomIndex = rand() % numGenes;
            int temp = population[i].path[j];
            population[i].path[j] = population[i].path[randomIndex];
            population[i].path[randomIndex] = temp;
        }
    }
}

// ���յ� ��� �� �� �� ����
void evaluatePopulation() {
    for (int i = 0; i < numCities; i++) {
        population[i].fitness = calculateFitness(&population[i]);
    }
}

// �귿 �� ���� ����� ����Ͽ� �θ� ����
struct Individual* selectParent() {
    double totalFitness = 0.0;
    for (int i = 0; i < numCities; i++) {
        totalFitness += population[i].fitness;
    }

    double randValue = (double)rand() / RAND_MAX;
    double partialSum = 0.0;

    for (int i = 0; i < numCities; i++) {
        partialSum += population[i].fitness / totalFitness;
        if (randValue <= partialSum) {
            return &population[i];
        }
    }

    return NULL;
}

// �� �θ� ���̿��� PMX�� �����Ͽ� �ڽ� ����
void crossover(struct Individual* parent1, struct Individual* parent2, struct Individual* child) {
    int startPos = rand() % numGenes;
    int endPos = rand() % numGenes;

    if (startPos > endPos) {
        int temp = startPos;
        startPos = endPos;
        endPos = temp;
    }

    for (int i = startPos; i <= endPos; i++) {
        child->path[i] = parent1->path[i];
    }

    int childIndex = endPos + 1;
    int parentIndex = childIndex;

    while (childIndex != startPos) {
        if (childIndex >= numGenes) {
            childIndex = 0;
        }
        if (parentIndex >= numGenes) {
            parentIndex = 0;
        }

        int city = parent2->path[parentIndex];

        int exists = 0;
        for (int i = startPos; i <= endPos; i++) {
            if (child->path[i] == city) {
                exists = 1;
                break;
            }
        }

        if (!exists) {
            child->path[childIndex] = city;
            childIndex++;
        }

        parentIndex++;
    }
}

// ���� �������� ����
void mutate(struct Individual* individual) {
    for (int i = 0; i < numGenes; i++) {
        double randValue = (double)rand() / RAND_MAX;
        if (randValue < 0.01) {
            int swapIndex = rand() % numGenes;
            int temp = individual->path[i];
            individual->path[i] = individual->path[swapIndex];
            individual->path[swapIndex] = temp;
        }
    }
}

// ��ȭ ���� ����: �� ���뿡�� �θ� ��ü �����ϰ� ���� �� �������̸� ���� ���� ��ü ��ü
void evolve() {
    for (int generation = 0; generation < numGenerations; generation++) {
        struct Individual* parent1 = selectParent();
        struct Individual* parent2 = selectParent();

        struct Individual child;
        child.path = (int*)malloc(numGenes * sizeof(int));
        crossover(parent1, parent2, &child);

        mutate(&child);

        double childFitness = calculateFitness(&child);

        double minFitness = population[0].fitness;
        int minIndex = 0;

        for (int i = 1; i < numCities; i++) {
            if (population[i].fitness < minFitness) {
                minFitness = population[i].fitness;
                minIndex = i;
            }
        }

        if (childFitness > minFitness) {
            free(population[minIndex].path);
            population[minIndex] = child;
        }
        else {
            free(child.path);
        }
    }
}

// ��ȭ �������� ã�� �ֻ��� ��� ���: ���� ���ø� ������ ���� ��ǥ�� ���Ͽ� ���
void printBestPath(int numCities) {
    double maxFitness = population[0].fitness;
    int maxIndex = 0;

    for (int i = 1; i < numCities; i++) {
        if (population[i].fitness > maxFitness) {
            maxFitness = population[i].fitness;
            maxIndex = i;
        }
    }

    FILE* file;
    if ((file = fopen("cities50.txt", "w")) == NULL) {
        printf("Failed to open file.\n");
        exit(1);
    }

    int startIndex = 0;
    int x = -380;
    int y = -350;

    fprintf(file, "%d %d\n", x, y);
    printf("%d %d\n", x, y);

    for (int i = 0; i < numGenes; i++) {
        int cityIndex = population[maxIndex].path[i];
        fprintf(file, "%d %d\n", cities[cityIndex].x, cities[cityIndex].y);
        printf("%d %d\n", cities[cityIndex].x, cities[cityIndex].y);
    }

    fclose(file);
}

int main() {
    srand(time(NULL));

    readCoordinates("cities50.txt");
    initializePopulation();
    evaluatePopulation();
    evolve();
    printBestPath(numCities);

    for (int i = 0; i < numCities; i++) {
        free(population[i].path);
    }
    free(population);
    free(cities);

    return 0;
}
