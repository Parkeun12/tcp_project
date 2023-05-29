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

int numCities = 50; //도시 수
int numGenerations = 1000; //진화 반복 수
int numGenes = 50; //유전자 수 

// 파일에서 도시의 좌표를 읽고 객체 배열에 저장
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

// 두 도시 간의 거리를 계산하기 위해 거리 공식 사용하여 거리 계산
double calculateDistance(struct City city1, struct City city2) {
    int dx = city1.x - city2.x;
    int dy = city1.y - city2.y;
    return sqrt(dx * dx + dy * dy);
}

// 적합도 계산: 이동한 전체 거리를 기반으로 하며, 거리가 짧을수록 높은 적합도 값
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

// 모집단을 무작위로 초기화
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

// 적합도 계산 및 평가 후 저장
void evaluatePopulation() {
    for (int i = 0; i < numCities; i++) {
        population[i].fitness = calculateFitness(&population[i]);
    }
}

// 룰렛 휠 선택 방식을 사용하여 부모 선택
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

// 두 부모 사이에서 PMX를 수행하여 자식 생성
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

// 스왑 돌연변이 수행
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

// 진화 과정 수행: 각 세대에서 부모 개체 선택하고 교차 및 돌연변이를 통해 약한 개체 대체
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

// 진화 과정에서 찾은 최상의 경로 출력: 시작 도시를 포함한 도시 좌표를 파일에 출력
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
