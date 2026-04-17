
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <cmath>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#define SCREEN_WIDTH (1920)
#define SCREEN_HEIGHT (1080)

#define WINDOW_TITLE "Space simulation"

constexpr double G_CONSTANT = 6.674e-11;
#define SMALL_MASS 1
#define KM *1000
struct vector {
  double x = 0;
  double y = 0;
  double z = 0;
};
class Object {
public:
  double mass;
  float radius;
  Vector2 pos;
  std::string name = "";
  bool anchored = false;
  vector speed = {0, 0};
  bool isequal(Object *other) {
    return this->mass == other->mass && this->radius == other->radius &&
           this->pos.y == other->pos.y && this->pos.x == other->pos.x &&
           this->name == other->name && this->anchored == other->anchored;
  }
};

class Plain {
public:
  Plain() {}
  void addElement(Object obj) { this->objects.push_back(obj); }
  vector getGravityForceFromPoint(vector position, double mass) {
    vector sum = {0, 0};
    for (Object obj : this->objects) {
      double disX = obj.pos.x - position.x;
      double disY = obj.pos.y - position.y;
      double r2 = std::pow(disY, 2) + std::pow(disX, 2);
      double r = sqrt(r2);
      vector normalizedForce = {disX / r, disY / r};
      double force = G_CONSTANT * ((obj.mass + mass) / r2);
      sum.x += force * normalizedForce.x;
      sum.y += force * normalizedForce.y;
    }
    return sum;
  }
  void applyMovement(double time) {
    for (int i = 0; i < this->objects.size(); i++) {
      auto &obj = this->objects.at(i);
      if (obj.anchored)
        continue;
      auto accel = this->getGravityForceFromItem(obj);
      obj.pos.x +=
          obj.speed.x * time + (0.5 * (accel.x / obj.mass) * time * time);
      obj.pos.y +=
          obj.speed.y * time + (0.5 * (accel.y / obj.mass) * time * time);

      obj.speed.x = obj.speed.x + (accel.x / obj.mass) * time;
      obj.speed.y = obj.speed.y + (accel.y / obj.mass) * time;
    }
  }
  vector getGravityForceFromItem(Object o) {
    auto position = o.pos;
    auto mass = o.mass;
    vector sum = {0, 0};
    for (Object obj : this->objects) {
      if (obj.isequal(&o)) {
        continue;
      }
      double disX = obj.pos.x - position.x;
      double disY = obj.pos.y - position.y;
      double r2 = std::pow(disY, 2) + std::pow(disX, 2);
      double r = sqrt(r2);
      vector normalizedForce = {disX / r, disY / r};
      double force = G_CONSTANT * ((obj.mass * mass) / r2);
      sum.x += force * normalizedForce.x;
      sum.y += force * normalizedForce.y;
    }
    return sum;
  }
  float max_width = SCREEN_WIDTH;
  float max_height = (SCREEN_HEIGHT / 100) * 90;

  std::vector<Object> objects;

private:
};

class Game {
public:
  bool paused = false;
  double timeMultiplier = 10;
  int scale = 50000; // 1 pixel is equal to "scale" km
  Game(std::string gameName) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, gameName.c_str());
    SetTargetFPS(60);
  }
  Plain *plain;
  int density = 10;
  int pointMass = 100;
  void applySpeeds() {
    if (paused) {
      return;
    }
    this->plain->applyMovement(timeMultiplier);
  }

  bool scaleEdit = false;
  bool scaleMult = false;
  void render() {
    float relativeXPos = 0;

    if (plain != nullptr) {
      Rectangle background;
      background.x = 0;
      background.y = (SCREEN_HEIGHT / 10);
      background.height = plain->max_height;
      background.width = plain->max_width;
      DrawRectangleRec(background, BLACK);
      auto center = vector((SCREEN_WIDTH / 2) + background.x,
                           (SCREEN_HEIGHT / 2 + background.y));
      for (auto obj : plain->objects) {
        Vector2 pos = {obj.pos.x / this->scale, obj.pos.y / this->scale};
        pos.x += center.x;
        pos.y += center.y;
        DrawCircleV(pos, obj.radius / this->scale, WHITE);
        DrawText((("Name: " + obj.name)).c_str(), pos.x, pos.y + 20, 10, GREEN);
        if (IsKeyDown(KEY_LEFT_CONTROL)) {
          int offset = 0;
          if (GetGestureDetected() == GESTURE_DRAG) {
            offset += 10;
          }
          if (CheckCollisionCircles(pos, obj.radius / this->scale,
                                    GetMousePosition(), 2)) {
            DrawText(("Mass: " + std::to_string(obj.mass)).c_str(), 20,
                     25 + 10 + offset, 10, BLACK);
            DrawText(("Radius: " + std::to_string(obj.radius)).c_str(), 20,
                     25 + 20 + offset, 10, BLACK);
            DrawText((("Name: " + obj.name)).c_str(), 20, 25 + 30 + offset, 10,
                     BLACK);
            auto force = this->plain->getGravityForceFromPoint(
                vector{obj.pos.x + obj.radius, obj.pos.y}, 0);
            auto f = sqrt((force.x * force.x) + (force.y * force.y));

            DrawText((("Surface gravity: " + std::to_string(f))).c_str(), 20,
                     25 + 40 + offset, 10, BLACK);
          }
        }
      }
      if (GetGestureDetected() == GESTURE_DRAG) {
        vector pos = {GetMousePosition().x - center.x,
                      GetMousePosition().y - center.y};
        pos.x *= scale;
        pos.y *= scale;
        auto force =
            this->plain->getGravityForceFromPoint(pos, this->pointMass);
        DrawCircle(GetMousePosition().x, GetMousePosition().y, 10, GREEN);
        DrawLineEx(GetMousePosition(),
                   {(float)(GetMousePosition().x + force.x * 10),
                    (float)(GetMousePosition().y + force.y * 10)},
                   3, WHITE);
        DrawCircle(GetMousePosition().x + force.x * 10,
                   GetMousePosition().y + force.y * 10, 5, BLUE);
        auto f = sqrt((force.x * force.x) + (force.y * force.y));
        DrawText(("Force: " + std::to_string(f)).c_str(), 20, 20, 10, BLACK);
      }
    }
    {
      DrawRectangleRec(Rectangle{relativeXPos, 0, 310, (SCREEN_HEIGHT / 10)},
                       GRAY);
      DrawRectangleRec(
          Rectangle{relativeXPos + 5, 5, 300, (SCREEN_HEIGHT / 10) - 10},
          WHITE);
      DrawText("Data", relativeXPos + 10, 10, 5, BLACK);
      relativeXPos += 310;
    }
    {
      DrawRectangleRec(Rectangle{relativeXPos, 0, 410, (SCREEN_HEIGHT / 10)},
                       GRAY);
      DrawRectangleRec(
          Rectangle{relativeXPos + 5, 5, 400, (SCREEN_HEIGHT / 10) - 10},
          WHITE);
      DrawText("Commands", relativeXPos + 10, 10, 5, BLACK);
      if (GuiButton({20 + relativeXPos, 35, 80, 30},
                    paused ? "Resume" : "Pause")) {
        this->paused = !this->paused;
      }
      if (GuiValueBox({140 + relativeXPos, 35, 80, 30}, "scale  ", &this->scale,
                      0, 1000000, scaleEdit)) {
        scaleEdit = !scaleEdit;
      }
      static int timeMult = (int)this->timeMultiplier;
      if (GuiValueBox({300 + relativeXPos, 35, 80, 30}, "simulated s/s",
                      &timeMult, 0, 1000000, scaleMult)) {
        scaleMult = !scaleMult;
      }
      this->timeMultiplier = timeMult;
      relativeXPos += 410;
    }
    {
      DrawRectangleRec(Rectangle{relativeXPos, 0, 310, (SCREEN_HEIGHT / 10)},
                       GRAY);
      DrawRectangleRec(
          Rectangle{relativeXPos + 5, 5, 300, (SCREEN_HEIGHT / 10) - 10},
          WHITE);
      DrawText("insertion", relativeXPos + 10, 10, 5, BLACK);

      static int newMass = 1000;
      static int newRadius = 200000;
      static int newX = 0;
      static int newY = 0;
      static bool newAnchored = false;
      static char newName[32] = "object";

      static bool massEdit = false;
      static bool radiusEdit = false;
      static bool xEdit = false;
      static bool yEdit = false;
      static bool nameEdit = false;

      float row1 = 20;
      float row2 = 55;
      float col = relativeXPos + 10;

      // row 1: name, mass, radius
      if (GuiTextBox({col, row1, 80, 25}, newName, 32, nameEdit))
        nameEdit = !nameEdit;
      if (GuiValueBox({col + 90, row1, 80, 25}, "mass", &newMass, 0, 1e9,
                      massEdit))
        massEdit = !massEdit;
      if (GuiValueBox({col + 180, row1, 80, 25}, "radius", &newRadius, 0, 1e8,
                      radiusEdit))
        radiusEdit = !radiusEdit;

      // row 2: x, y, anchored, add button
      if (GuiValueBox({col, row2, 60, 25}, "x", &newX, -1e8, 1e8, xEdit))
        xEdit = !xEdit;
      if (GuiValueBox({col + 70, row2, 60, 25}, "y", &newY, -1e8, 1e8, yEdit))
        yEdit = !yEdit;
      GuiCheckBox({col + 140, row2, 25, 25}, "anchor", &newAnchored);

      if (GuiButton({col + 175, row2, 55, 25}, "Add")) {
        Object obj;
        obj.name = newName;
        obj.mass = newMass;
        obj.radius = newRadius;
        obj.pos.x = newX;
        obj.pos.y = newY;
        obj.anchored = newAnchored;
        this->plain->addElement(obj);
      }

      relativeXPos += 310;
    }
  }
};

int main(void) {
  Plain piano;

  Game game("gravity simulator");
  game.plain = &piano;
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    game.applySpeeds();
    game.render();

    EndDrawing();
  }
  return 0;
}
