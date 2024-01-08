#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <stack>
#include <queue>
#include <utility>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
using namespace std;

#define buttons_path "buttons.png"
#define font_path "OpenSans.ttf"

int cellSize = 30;
int cellsNumbersX = 15;
int cellsNumbersY = 15;
int gridWidth = cellsNumbersX * cellSize;
int gridHeight = cellsNumbersY * cellSize;

map<pair<int, int>, int> cellType;
map<pair<int, int>, int> cellWeight;
// moving 1 cell costs 1 unit by default;
vector<pair<string, pair<int, int>>> weightPoints;

int dx[] = {0, cellSize, 0, -cellSize};
int dy[] = {-cellSize, 0, cellSize, 0};
map<pair<int, int>, bool> visited;
map<pair<int, int>, bool> isInRoute;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* buttons;
SDL_Point mousePoint;
SDL_Rect startPoint;
SDL_Rect finishPoint;

class Button {
 public:
  SDL_Texture* tex;
  SDL_Rect sRect{}, dRect{};
  bool isClicked;
  Button();
  ~Button();
  void draw();
  void update(int oy, int ny);
};

bool isCell(int x, int y) {
  return (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight);
}

void removeRoute() {
  visited.clear();
  isInRoute.clear();
  SDL_SetRenderDrawColor(renderer, 15, 15, 15, 255);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, 75, 75, 75, 255);
  for (int i = 0; i <= gridWidth; i += cellSize) {
    SDL_RenderDrawLine(renderer, i, 0, i, gridHeight);
  }
  for (int i = 0; i <= gridHeight; i += cellSize) {
    SDL_RenderDrawLine(renderer, 0, i, gridWidth, i);
  }
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
  SDL_RenderFillRect(renderer, &startPoint);
  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
  SDL_RenderFillRect(renderer, &finishPoint);
  for (int i = 0; i < gridWidth; i += cellSize) {
    for (int j = 0; j < gridHeight; j += cellSize) {
      if (cellType[{i, j}] == 1 || cellType[{i, j}] == 2) {
        continue;
      }
      if (cellType[{i, j}] == -1) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect obstacle = {i, j, cellSize, cellSize};
        SDL_RenderFillRect(renderer, &obstacle);
      }
      else if (isInRoute[{i, j}]) {
        SDL_SetRenderDrawColor(renderer, 150, 150, 0, 255);
        SDL_Rect neighbor = {i, j, cellSize, cellSize};
        SDL_RenderFillRect(renderer, &neighbor);
      }
    }
  }
  SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
  SDL_Init(SDL_INIT_EVERYTHING);
  IMG_Init(IMG_INIT_PNG);
  TTF_Init();
  SDL_CreateWindowAndRenderer(gridWidth + 300, gridHeight + 100, 0, &window, &renderer);
  SDL_SetWindowTitle(window, "Path Finder");
  buttons = IMG_LoadTexture(renderer, buttons_path);
  TTF_Font* font = TTF_OpenFont(font_path, 60);

  for (int i = 0; i < gridWidth; i += cellSize) {
    for (int j = 0; j < gridHeight; j += cellSize) {
      cellWeight[{i, j}] = 1;
    }
  }

  startPoint = {0, 0, cellSize, cellSize};
  cellType[{0, 0}] = 1;
  finishPoint = {(cellsNumbersX - 1) * cellSize, (cellsNumbersY - 1) * cellSize, cellSize, cellSize};
  cellType[{(cellsNumbersX - 1) * cellSize, (cellsNumbersY - 1) * cellSize}] = 2;

  Button weightButton;
  weightButton.sRect.y = 0;
  weightButton.dRect.y = 25;

  Button obstacleButton;
  obstacleButton.sRect.y = 330;
  obstacleButton.dRect.y = 130;

  Button startPointButton;
  startPointButton.sRect.y = 660;
  startPointButton.dRect.y = 235;

  Button finishPointButton;
  finishPointButton.sRect.y = 990;
  finishPointButton.dRect.y = 340;

  Button DFSButton;
  DFSButton.sRect.y = 1320;
  DFSButton.dRect.x = 15;
  DFSButton.dRect.y = gridHeight + 10;

  Button BFSButton;
  BFSButton.sRect.y = 1650;
  BFSButton.dRect.x = 15 + 225 + 15;
  BFSButton.dRect.y = gridHeight + 10;

  Button DijkstraButton;
  DijkstraButton.sRect.y = 1980;
  DijkstraButton.dRect.x = 15 + 225 + 15 + 225 + 15;
  DijkstraButton.dRect.y = gridHeight + 10;

  SDL_Surface* resetSurf = TTF_RenderText_Solid(font, "Press 0 to reset the grid.", {255, 255, 255});
  SDL_Texture* resetText = SDL_CreateTextureFromSurface(renderer, resetSurf);
  SDL_FreeSurface(resetSurf);
  SDL_Rect resetDest = {gridWidth + 15, gridHeight - 35, 250, 35};

  bool quit = false;
  while (!quit) {
    Uint32 startTicks = SDL_GetTicks();

    SDL_Event event;
    int x, y;
    SDL_GetMouseState(&x, &y);
    mousePoint = {x, y};
    int nx = x / cellSize * cellSize;
    int ny = y / cellSize * cellSize;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
          if (event.button.button == SDL_BUTTON_LEFT) {
            if (isCell(nx, ny)) {
              if (startPointButton.isClicked && cellType[{nx, ny}] == 0) {
                cellType[{startPoint.x, startPoint.y}] = 0;
                startPoint.x = nx;
                startPoint.y = ny;
                cellType[{startPoint.x, startPoint.y}] = 1;
              }
              else if (finishPointButton.isClicked && cellType[{nx, ny}] == 0) {
                cellType[{finishPoint.x, finishPoint.y}] = 0;
                finishPoint.x = nx;
                finishPoint.y = ny;
                cellType[{finishPoint.x, finishPoint.y}] = 2;
              }
              else if (obstacleButton.isClicked) {
                if (cellWeight[{nx, ny}] > 1 && cellType[{nx, ny}] == 0) {
                  int idx = -1;
                  for (int i = 0; i < (int)weightPoints.size(); ++i) {
                    if (weightPoints[i].second.first == nx && weightPoints[i].second.second == ny) {
                      idx = i;
                      break;
                    }
                  }
                  weightPoints.erase(weightPoints.begin() + idx);
                  cellWeight[{nx, ny}] = 1;
                }
                if (cellType[{nx, ny}] == 0) {
                  cellType[{nx, ny}] = -1;
                }
                else if (cellType[{nx, ny}] == - 1) {
                  cellType[{nx, ny}] = 0;
                }
              }
              else if (weightButton.isClicked && cellType[{nx, ny}] == 0) {
                ++cellWeight[{nx, ny}];
                if (cellWeight[{nx, ny}] != 2) {
                  for (auto &weight : weightPoints) {
                    if (weight.second.first == nx && weight.second.second == ny) {
                      weight.first = to_string(cellWeight[{nx, ny}]);
                      break;
                    }
                  }
                } else {
                  weightPoints.push_back({to_string(cellWeight[{nx, ny}]), {nx, ny}});
                }
              }
            }
            else {
              if (SDL_PointInRect(&mousePoint, &weightButton.dRect)) {
                if (weightButton.isClicked) {
                  weightButton.isClicked = false;
                }
                else {
                  weightButton.isClicked = true;
                  obstacleButton.isClicked = false;
                  startPointButton.isClicked = false;
                  finishPointButton.isClicked = false;
                }
              }
              else if (SDL_PointInRect(&mousePoint, &obstacleButton.dRect)) {
                if (obstacleButton.isClicked) {
                  obstacleButton.isClicked = false;
                }
                else {
                  obstacleButton.isClicked = true;
                  weightButton.isClicked = false;
                  startPointButton.isClicked = false;
                  finishPointButton.isClicked = false;
                }
              }
              else if (SDL_PointInRect(&mousePoint, &startPointButton.dRect)) {
                if (startPointButton.isClicked) {
                  startPointButton.isClicked = false;
                }
                else {
                  startPointButton.isClicked = true;
                  weightButton.isClicked = false;
                  obstacleButton.isClicked = false;
                  finishPointButton.isClicked = false;
                }
              }
              else if (SDL_PointInRect(&mousePoint, &finishPointButton.dRect)) {
                if (finishPointButton.isClicked) {
                  finishPointButton.isClicked = false;
                }
                else {
                  finishPointButton.isClicked = true;
                  weightButton.isClicked = false;
                  obstacleButton.isClicked = false;
                  startPointButton.isClicked = false;
                }
              }
              else if (SDL_PointInRect(&mousePoint, &DFSButton.dRect)) {
                removeRoute();
                DFSButton.isClicked = true;
                BFSButton.isClicked = false;
                DijkstraButton.isClicked = false;
                stack<pair<pair<int, int>, pair<int, int>>> dfs;
                map<pair<int, int>, pair<int, int>> parent;
                dfs.push({{startPoint.x, startPoint.y}, {startPoint.x, startPoint.y}});
                while (!dfs.empty()) {
                  pair<int, int> cur = dfs.top().first;
                  pair<int, int> curPar = dfs.top().second;
                  dfs.pop();
                  int cx = cur.first;
                  int cy = cur.second;
                  int cPx = curPar.first;
                  int cPy = curPar.second;
                  if (!isCell(cx, cy) || visited[{cx, cy}] || cellType[{cx, cy}] == -1) {
                    continue;
                  }
                  visited[{cx, cy}] = true;
                  parent[{cx, cy}] = {cPx, cPy};
                  SDL_Rect neighbor = {cx, cy, cellSize, cellSize};
                  SDL_SetRenderDrawColor(renderer, 150, 150, 0, 255);
                  SDL_RenderFillRect(renderer, &neighbor);
                  if (cx == finishPoint.x && cy == finishPoint.y) {
                    break;
                  }
                  for (int i = 0; i < 4; ++i) {
                    int adjX = cx + dx[i];
                    int adjY = cy + dy[i];
                    dfs.push({{adjX, adjY}, {cx, cy}});
                  }
                  SDL_Delay(15);
                  SDL_RenderPresent(renderer);
                }
                if (!visited[{finishPoint.x, finishPoint.y}]) {
                  cout << "NO" << endl;
                }
                else {
                  cout << "YES" << endl;
                  pair<int, int> start = {startPoint.x, startPoint.y};
                  pair<int, int> finish = {finishPoint.x, finishPoint.y};
                  int steps = 0;
                  vector<pair<int, int>> path;
                  path.push_back(finish);
                  pair<int, int> cur = finish;
                  while (cur != start) {
                    path.push_back(parent[cur]);
                    cur = parent[cur];
                  }
                  reverse(path.begin(), path.end());
                  for (auto &step: path) {
                    isInRoute[step] = true;
                    cout << "#" << ++steps << "   X: " << step.first << "   Y: " << step.second << endl;
                  }
                }
              }
              else if (SDL_PointInRect(&mousePoint, &BFSButton.dRect)) {
                removeRoute();
                DFSButton.isClicked = false;
                BFSButton.isClicked = true;
                DijkstraButton.isClicked = false;
                queue<pair<pair<int, int>, pair<int, int>>> bfs;
                map<pair<int, int>, pair<int, int>> parent;
                bfs.push({{startPoint.x, startPoint.y}, {startPoint.x, startPoint.y}});
                while (!bfs.empty()) {
                  pair<int, int> cur = bfs.front().first;
                  pair<int, int> curPar = bfs.front().second;
                  bfs.pop();
                  int cx = cur.first;
                  int cy = cur.second;
                  int cPx = curPar.first;
                  int cPy = curPar.second;
                  if (!isCell(cx, cy) || visited[{cx, cy}] || cellType[{cx, cy}] == -1) {
                    continue;
                  }
                  visited[{cx, cy}] = true;
                  parent[{cx, cy}] = {cPx, cPy};
                  SDL_Rect neighbor = {cx, cy, cellSize, cellSize};
                  SDL_SetRenderDrawColor(renderer, 150, 150, 0, 255);
                  SDL_RenderFillRect(renderer, &neighbor);
                  if (cx == finishPoint.x && cy == finishPoint.y) {
                    break;
                  }
                  for (int i = 0; i < 4; ++i) {
                    int adjX = cx + dx[i];
                    int adjY = cy + dy[i];
                    bfs.push({{adjX, adjY}, {cx, cy}});
                  }
                  SDL_Delay(15);
                  SDL_RenderPresent(renderer);
                }
                if (!visited[{finishPoint.x, finishPoint.y}]) {
                  cout << "NO" << endl;
                }
                else {
                  cout << "YES" << endl;
                  pair<int, int> start = {startPoint.x, startPoint.y};
                  pair<int, int> finish = {finishPoint.x, finishPoint.y};
                  int steps = 0;
                  vector<pair<int, int>> path;
                  path.push_back(finish);
                  pair<int, int> cur = finish;
                  while (cur != start) {
                    path.push_back(parent[cur]);
                    cur = parent[cur];
                  }
                  reverse(path.begin(), path.end());
                  for (auto &step: path) {
                    isInRoute[step] = true;
                    cout << "#" << ++steps << "   X: " << step.first << "   Y: " << step.second << endl;
                  }
                }
              }
              else if (SDL_PointInRect(&mousePoint, &DijkstraButton.dRect)) {
                removeRoute();
                DFSButton.isClicked = false;
                BFSButton.isClicked = false;
                DijkstraButton.isClicked = true;
                priority_queue<pair<int, pair<pair<int, int>, pair<int, int>>>, vector<pair<int, pair<pair<int, int>, pair<int, int>>>>, greater<>> dijkstra;
                map<pair<int, int>, pair<int, int>> parent;
                dijkstra.push({0, {{startPoint.x, startPoint.y}, {startPoint.x, startPoint.y}}});
                vector<vector<int>> dist(905, vector<int>(905, INT_MAX));
                dist[startPoint.x][startPoint.y] = 0;
                while (!dijkstra.empty()) {
                  pair<int, int> cur = dijkstra.top().second.first;
                  pair<int, int> curPar = dijkstra.top().second.second;
                  int d = dijkstra.top().first;
                  dijkstra.pop();
                  int cx = cur.first;
                  int cy = cur.second;
                  int cPx = curPar.first;
                  int cPy = curPar.second;
                  visited[{cx, cy}] = true;
                  parent[{cx, cy}] = {cPx, cPy};
                  SDL_Rect neighbor = {cx, cy, cellSize, cellSize};
                  SDL_SetRenderDrawColor(renderer, 150, 150, 0, 255);
                  SDL_RenderFillRect(renderer, &neighbor);
                  if (cx == finishPoint.x && cy == finishPoint.y) {
                    break;
                  }
                  for (int i = 0; i < 4; ++i) {
                    int adjX = cx + dx[i];
                    int adjY = cy + dy[i];
                    if (!isCell(adjX, adjY) || cellType[{adjX, adjY}] == -1) {
                      continue;
                    }
                    if (dist[cx][cy] + cellWeight[{adjX, adjY}] < dist[adjX][adjY]) {
                      dist[adjX][adjY] = dist[cx][cy] + cellWeight[{adjX, adjY}];
                      dijkstra.push({dist[adjX][adjY], {{adjX, adjY}, {cx, cy}}});
                    }
                  }
                  SDL_Delay(15);
                  SDL_RenderPresent(renderer);
                }
                if (!visited[{finishPoint.x, finishPoint.y}]) {
                  cout << "NO" << endl;
                }
                else {
                  cout << "YES" << endl;
                  pair<int, int> start = {startPoint.x, startPoint.y};
                  pair<int, int> finish = {finishPoint.x, finishPoint.y};
                  int steps = 0;
                  vector<pair<int, int>> path;
                  path.push_back(finish);
                  pair<int, int> cur = finish;
                  while (cur != start) {
                    path.push_back(parent[cur]);
                    cur = parent[cur];
                  }
                  reverse(path.begin(), path.end());
                  for (auto &step: path) {
                    isInRoute[step] = true;
                    cout << "#" << ++steps << "   X: " << step.first << "   Y: " << step.second << endl;
                  }
                }
              }
            }
          }
          else if (event.button.button == SDL_BUTTON_RIGHT) {
            if (isCell(nx, ny)) {
              if (weightButton.isClicked) {
                if (cellType[{nx, ny}] == 0 && cellWeight[{nx, ny}] > 1) {
                  --cellWeight[{nx, ny}];
                  if (cellWeight[{nx, ny}] == 1) {
                    for (int i = 0; i < (int)weightPoints.size(); ++i) {
                      if (weightPoints[i].second.first == nx && weightPoints[i].second.second == ny) {
                        weightPoints.erase(weightPoints.begin() + i);
                        break;
                      }
                    }
                  }
                  else {
                    for (auto &weight: weightPoints) {
                      if (weight.second.first == nx && weight.second.second == ny) {
                        weight.first = to_string(cellWeight[{nx, ny}]);
                        break;
                      }
                    }
                  }
                }
              }
            }
          }
          break;
        case SDL_KEYDOWN:
          if (event.key.keysym.scancode == SDL_SCANCODE_0) {
            weightPoints.clear();
            cellType.clear();
            for (int i = 0; i < gridWidth; i += cellSize) {
              for (int j = 0; j < gridHeight; j += cellSize) {
                cellWeight[{i, j}] = 1;
                cellType[{i, j}] = 0;
              }
            }
            visited.clear();
            isInRoute.clear();
            DFSButton.isClicked = false;
            BFSButton.isClicked = false;
            DijkstraButton.isClicked = false;
            startPoint.x = 0;
            startPoint.y = 0;
            cellType[{0, 0}] = 1;
            finishPoint.x = (cellsNumbersX - 1) * cellSize;
            finishPoint.y = (cellsNumbersY - 1) * cellSize;
            cellType[{(cellsNumbersX - 1) * cellSize, (cellsNumbersY - 1) * cellSize}] = 2;
          }
          break;
        case SDL_QUIT:
          quit = true;
          break;
      }
    }

    weightButton.update(0, 165);
    obstacleButton.update(330, 495);
    startPointButton.update(660, 825);
    finishPointButton.update(990, 1155);
    DFSButton.update(1320, 1485);
    BFSButton.update(1650, 1815);
    DijkstraButton.update(1980, 2145);

    SDL_SetRenderDrawColor(renderer, 15, 15, 15, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 75, 75, 75, 255);
    for (int i = 0; i <= gridWidth; i += cellSize) {
      SDL_RenderDrawLine(renderer, i, 0, i, gridHeight);
    }
    for (int i = 0; i <= gridHeight; i += cellSize) {
      SDL_RenderDrawLine(renderer, 0, i, gridWidth, i);
    }
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &startPoint);
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &finishPoint);
    for (int i = 0; i < gridWidth; i += cellSize) {
      for (int j = 0; j < gridHeight; j += cellSize) {
        if (cellType[{i, j}] == 1 || cellType[{i, j}] == 2) {
          continue;
        }
        if (cellType[{i, j}] == -1) {
          SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
          SDL_Rect obstacle = {i, j, cellSize, cellSize};
          SDL_RenderFillRect(renderer, &obstacle);
        }
        else if (isInRoute[{i, j}]) {
          SDL_SetRenderDrawColor(renderer, 150, 150, 0, 255);
          SDL_Rect neighbor = {i, j, cellSize, cellSize};
          SDL_RenderFillRect(renderer, &neighbor);
        }
      }
    }

    weightButton.draw();
    obstacleButton.draw();
    startPointButton.draw();
    finishPointButton.draw();
    DFSButton.draw();
    BFSButton.draw();
    DijkstraButton.draw();

    for (auto &weight: weightPoints) {
      SDL_Surface* text_surf = TTF_RenderText_Solid(font, weight.first.c_str(), {255, 255, 255});
      SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, text_surf);
      SDL_Rect textDest = {weight.second.first, weight.second.second, cellSize, cellSize};
      SDL_RenderCopy(renderer, text, nullptr, &textDest);
      SDL_DestroyTexture(text);
      SDL_FreeSurface(text_surf);
    }

    SDL_RenderCopy(renderer, resetText, nullptr, &resetDest);

    SDL_RenderPresent(renderer);
    Uint32 frameTicks = SDL_GetTicks() - startTicks;
    if (frameTicks < 1000 / 60) {
      SDL_Delay(1000 / 60 - frameTicks);
    }
  }

  SDL_DestroyTexture(resetText);
  SDL_DestroyTexture(buttons);
  SDL_DestroyRenderer(renderer);
  renderer = nullptr;
  SDL_DestroyWindow(window);
  window = nullptr;
  IMG_Quit();
  TTF_Quit();
  SDL_Quit();
  return 0;
}

Button::Button() {
  tex = buttons;
  sRect.x = 0;
  sRect.w = 350;
  sRect.h = 118;
  dRect.x = gridWidth + 25;
  dRect.w = 225;
  dRect.h = 75;
  isClicked = false;
}
Button::~Button() {
  SDL_DestroyTexture(tex);
}
void Button::draw() {
  SDL_RenderCopy(renderer, tex, &sRect, &dRect);
}
void Button::update(int oy, int ny) {
  if (SDL_PointInRect(&mousePoint, &dRect) || isClicked) {
    sRect.y = ny;
  } else {
    sRect.y = oy;
  }
}
