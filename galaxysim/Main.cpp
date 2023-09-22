#include <raylib.h>
#include <raymath.h>
#include <vector>

using std::vector;

constexpr float galaxyRadius = 250;
constexpr size_t numStars = 500;
constexpr size_t numGasClumps = 500;
constexpr float simulationSpeed = 10.0f;
Camera camera = { { 0, 0, -galaxyRadius * 2 }, Vector3Zero(), {0, 1, 0}, 90.0f, CAMERA_PERSPECTIVE};

constexpr float G = 4;

// Solar units
float StarRadius(float mass)
{
    return powf(mass, 0.8f);
}

Texture starTexture;
Texture gasTexture;

struct Body
{
    Body(Vector3 position, Vector3 velocity, float mass) :
        position(position), velocity(velocity), mass(mass) {}

    Vector3 position;
    Vector3 velocity;
    float mass;

    virtual void Draw2D() const = 0;
    virtual void Draw() const = 0;
};

struct Star : public Body
{
    Star(Vector3 position, Vector3 velocity, float mass) :
        Body(position, velocity, mass), color(WHITE), radius(StarRadius(mass)) {}

    Star(Vector3 position, Vector3 velocity, float mass, Color color) :
        Body(position, velocity, mass), color(color), radius(StarRadius(mass)) {}

    Color color;
    float radius;

    void Draw2D() const override
    {
        DrawPixelV(GetWorldToScreen(position, camera), color);
    }

    void Draw() const override
    {
#if 0
        DrawLine3D(Vector3Zero(), position, MAGENTA); // Line from center (used for locating)
#endif
#if 0
        DrawLine3D(position, Vector3Add(position, velocity), GREEN); // Velocity
#endif
        DrawBillboard(camera, starTexture, position, radius, color);
    }
};

struct GasClump : public Body
{
    GasClump(Vector3 position, Vector3 velocity, float mass, Color color) :
        Body(position, velocity, mass), color(color) {}

    Color color;

    void Draw2D() const override {}

    void Draw() const override
    {
        DrawBillboard(camera, gasTexture, position, mass * 8, color);
    }
};

float RandBetween(float min, float max)
{
    return ((float)rand() / (float)RAND_MAX) * (max - min) + min;
}

Color ColorLerp(Color a, Color b, float amount)
{
    return {
        (unsigned char)Lerp(a.r, b.r, amount),
        (unsigned char)Lerp(a.g, b.g, amount),
        (unsigned char)Lerp(a.b, b.b, amount),
        (unsigned char)Lerp(a.a, b.a, amount)
    };
}

Star&& RandomStar()
{
    float angle = RandBetween(0.0f, 2.0f * PI);
    float distance = RandBetween(15, galaxyRadius);
    float t = distance / galaxyRadius;
    float eccentricity = RandBetween(-PI / 8, PI / 8) * (1 - t);
    float mass = Lerp(8.0f, 0.5f, t); // 1 per billion
    Vector3 startPosition = { 0, distance, 0 };
    Vector3 offsetFromDisc = Vector3RotateByAxisAngle(startPosition, { 1, 0, 0 }, eccentricity);
    Vector3 aroundCenter = Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle);
    Vector3 position = aroundCenter;
    Vector3 velocity = Vector3RotateByAxisAngle(Vector3Scale(Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle), 0.25f), { 0, 0, 1 }, PI / 3);
    return Star(position, velocity, mass);
}

GasClump&& RandomGasClump()
{
    float angle = RandBetween(0.0f, 2.0f * PI);
    float distance = RandBetween(20, galaxyRadius);
    float t = distance / galaxyRadius;
    float eccentricity = RandBetween(-PI / 8, PI / 8) * (1 - t);
    float mass = Lerp(8, 0.5f, t); // 1 per billion
    Vector3 startPosition = { 0, distance, 0 };
    Vector3 offsetFromDisc = Vector3RotateByAxisAngle(startPosition, { 1, 0, 0 }, eccentricity);
    Vector3 aroundCenter = Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle);
    Vector3 position = aroundCenter;
    Vector3 velocity = Vector3RotateByAxisAngle(Vector3Scale(Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle), 0.25f), { 0, 0, 1 }, PI / 3);
    return GasClump(position, velocity, mass, ColorLerp(BLUE, VIOLET, t));
}

int main()
{
    InitWindow(720, 720, "Galaxy Sim");

    SetTargetFPS(0);

    {
        Image img = GenImageColor(2, 2, WHITE);
        starTexture = LoadTextureFromImage(img);
        UnloadImage(img);

        img = GenImageColor(2, 2, { 255, 255, 255, 127 });
        gasTexture = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    vector<Body*> bodies;
    bodies.reserve(numStars);

    srand(2);

    // Black hole
    Star* blackHole = new Star(Vector3Zero(), Vector3Zero(), 4.154e4f, { 0, 0, 0, 255 });
    blackHole->radius = 15.0f;
    bodies.push_back(blackHole);

    while (bodies.size() < numStars)
    {
        bodies.push_back(new Star(RandomStar()));
    }

    while (bodies.size() < numStars + numGasClumps)
    {
        bodies.push_back(new GasClump(RandomGasClump()));
    }

    bool isSimulationPaused = false;

    while (!WindowShouldClose())
    {
#if 1
        if (isSimulationPaused)
        {
            UpdateCamera(&camera, CAMERA_ORBITAL);
        }
        else
        {
            camera.position = { 0, 0, -galaxyRadius * 2 };
        }
#endif

        if (IsKeyPressed(KEY_SPACE))
        {
            isSimulationPaused = !isSimulationPaused;
        }

        if (!isSimulationPaused)
        {
            float dt = GetFrameTime() * simulationSpeed;

            for (size_t i = 0; i < bodies.size(); ++i)
            {
                Body* a = bodies[i];
                for (size_t j = i + 1; j < bodies.size(); ++j)
                {
                    Body* b = bodies[j];

                    float distance = Vector3Distance(a->position, b->position);

                    if (distance < 15.0f)
                    {
                        continue;
                    }

                    float gravity = (G * a->mass * b->mass) / (distance * distance);

                    Vector3 direction = Vector3Normalize(Vector3Subtract(b->position, a->position));
                    Vector3 force = Vector3Scale(direction, gravity);
                    Vector3 accelerationA = Vector3Scale(force, dt / a->mass);
                    Vector3 accelerationB = Vector3Scale(force, -dt / b->mass);

                    a->velocity = Vector3Add(a->velocity, accelerationA);
                    b->velocity = Vector3Add(b->velocity, accelerationB);
                }
            }

            for (Body* body : bodies)
            {
                body->position = Vector3Subtract(Vector3Add(body->position, Vector3Scale(body->velocity, dt)), blackHole->position);
                if (Vector3Distance(Vector3Zero(), body->position) > galaxyRadius * 2)
                {
                    if (Star* star = dynamic_cast<Star*>(body))
                    {
                        *star = RandomStar();
                    }
                    else if (GasClump* gasClump = dynamic_cast<GasClump*>(body))
                    {
                        *gasClump = RandomGasClump();
                    }
                }
            }
        }

        BeginDrawing();

            ClearBackground(BLACK);

            for (const Body* body : bodies)
            {
                body->Draw2D();
            }

            BeginMode3D(camera);
            for (const Body* body : bodies)
            {
                body->Draw();
            }
            EndMode3D();

            DrawFPS(0,0);

            if (isSimulationPaused)
            {
                DrawText("PAUSED", 0, 30, 8, WHITE);
            }

        EndDrawing();
    }

    UnloadTexture(starTexture);
    UnloadTexture(gasTexture);

    CloseWindow();
}
