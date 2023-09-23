#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <thread>

constexpr Color amyblue   = {  38, 144, 252, 255 };
constexpr Color amypurple = { 116,  26, 248, 255 };
constexpr Color amyhair[4] = {
    {  32, 142, 249, 255 },
    {  32,  93, 255, 255 },
    { 107,  28, 252, 255 },
    { 121,  23, 207, 255 },
};
constexpr Color amyfur    = {  41,  53,  48, 255 };
constexpr Color amyskin   = {  29,  25,  40, 255 };

using std::vector;

constexpr float galaxyRadius = 250;
constexpr size_t numStars = 2000;
constexpr size_t numGasClumps = 200;
constexpr size_t numDustClouds = 250;
constexpr size_t numDarkBodies = 200;
constexpr size_t numBodies = 1 + numStars + numGasClumps + numDustClouds + numDarkBodies;
constexpr float simulationSpeed = 1.0f;
Camera camera = { { 0, 0, -galaxyRadius * 1.5f }, Vector3Zero(), {0, 1, 0}, 120.0f, CAMERA_PERSPECTIVE};

constexpr float G = 9;

// Solar units
float StarRadius(float mass)
{
    return powf(mass, 0.8f);
}

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

Texture starTexture;
Texture gasTexture;
Texture dustTexture;

struct Body
{
    Body(Vector3 position, Vector3 velocity, float mass) :
        position(position), velocity(velocity), mass(mass) {}

    Vector3 position;
    Vector3 velocity;
    float mass;

    virtual void Draw2D() const = 0;
    virtual void Draw() const = 0;

    virtual void Randomize() = 0;
};

struct Star : public Body
{
    Star() :
        Body(Vector3Zero(), Vector3Zero(), 0.0f), color(BLACK), radius(0.0f) {}

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
        if (IsKeyDown(KEY_V))
        {
            DrawLine3D(position, Vector3Add(position, Vector3Scale(velocity, GetFrameTime() * simulationSpeed)), { 0, 127, 0, 64 }); // Velocity
        }
#endif
        //BeginBlendMode(BLEND_ADD_COLORS);
        DrawBillboard(camera, starTexture, position, radius, color);
        //EndBlendMode();
    }

    void Randomize() override
    {
        float angle = RandBetween(0.0f, 2.0f * PI);
        float distance = RandBetween(15, galaxyRadius);
        float t = distance / galaxyRadius;
        float eccentricity = RandBetween(-PI / 3, PI / 3) * (1 - t);
        Vector3 startPosition = { 0, distance, 0 };
        Vector3 offsetFromDisc = Vector3RotateByAxisAngle(startPosition, { 1, 0, 0 }, eccentricity);
        Vector3 aroundCenter = Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle);

        position = aroundCenter;
#if 1
        velocity = Vector3RotateByAxisAngle(Vector3Scale(Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle), 0.25f), { 0, 0, 1 }, PI / 3);
#elif 1
        velocity = { RandBetween(-10, 10), RandBetween(-10, 10), RandBetween(-1, 1) };
#else
        velocity = Vector3Zero();
#endif
        mass = Lerp(8.0f, 0.5f, t);
        radius = StarRadius(mass);
        color = ColorLerp(ColorLerp(amyblue, WHITE, t), ColorLerp(WHITE, amypurple, t), t);
    }
};

struct GasClump : public Body
{
    GasClump() :
        Body(Vector3Zero(), Vector3Zero(), 0.0f) {}

    GasClump(Vector3 position, Vector3 velocity, float mass, Color color) :
        Body(position, velocity, mass) {}

    void Draw2D() const override {}

    void Draw() const override
    {
        //BeginBlendMode(BLEND_ADDITIVE);
        float t = Clamp(Vector3Length(position) / galaxyRadius, 0, 1);
        Color color = ColorLerp(
            ColorLerp(ColorLerp(     WHITE, amyhair[0], t), ColorLerp(amyhair[0], amyhair[1], t), t),
            ColorLerp(ColorLerp(amyhair[1], amyhair[2], t), ColorLerp(amyhair[2], amyhair[3], t), t), t);
        DrawBillboard(camera, gasTexture, position, mass * 4, color);
        //EndBlendMode();
    }

    void Randomize() override
    {
        float angle = RandBetween(0.0f, 2.0f * PI);
        float distance = RandBetween(20, galaxyRadius);
        float t = distance / galaxyRadius;
        float eccentricity = RandBetween(-PI / 3, PI / 3) * (1 - t);
        Vector3 startPosition = { 0, distance, 0 };
        Vector3 offsetFromDisc = Vector3RotateByAxisAngle(startPosition, { 1, 0, 0 }, eccentricity);
        Vector3 aroundCenter = Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle);

        position = aroundCenter;
#if 1
        velocity = Vector3RotateByAxisAngle(Vector3Scale(Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle), 0.25f), { 0, 0, 1 }, PI / 3);
#elif 1
        velocity = { RandBetween(-10, 10), RandBetween(-10, 10), RandBetween(-1, 1) };
#else
        velocity = Vector3Zero();
#endif
        mass = Lerp(10, 0.5f, t);
    }
};

struct DustCloud : public Body
{
    DustCloud() :
        Body(Vector3Zero(), Vector3Zero(), 0.0f) {}

    DustCloud(Vector3 position, Vector3 velocity, float mass) :
        Body(position, velocity, mass) {}

    void Draw2D() const override {}

    void Draw() const override
    {
        //BeginBlendMode(BLEND_MULTIPLIED);
        float t = Clamp(Vector3Length(position) / galaxyRadius, 0.0f, 1.0f);
        DrawBillboard(camera, dustTexture, position, mass * 32, ColorLerp(amyfur, amyskin, t));
        //EndBlendMode();
    }

    void Randomize() override
    {
        float angle = RandBetween(0.0f, 2.0f * PI);
        float distance = RandBetween(galaxyRadius / 2, galaxyRadius);
        float t = distance / galaxyRadius;
        float eccentricity = RandBetween(-PI / 3, PI / 3) * (1 - t);
        Vector3 startPosition = { 0, distance, 0 };
        Vector3 offsetFromDisc = Vector3RotateByAxisAngle(startPosition, { 1, 0, 0 }, eccentricity);
        Vector3 aroundCenter = Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle);

        position = aroundCenter;
#if 1
        velocity = Vector3RotateByAxisAngle(Vector3Scale(Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle), 0.25f), { 0, 0, 1 }, PI / 3);
#elif 1
        velocity = { RandBetween(-10, 10), RandBetween(-10, 10), RandBetween(-1, 1) };
#else
        velocity = Vector3Zero();
#endif
        mass = Lerp(3.0f, 1.0f, t);
    }
};

struct DarkBody : public Body
{
    DarkBody() :
        Body(Vector3Zero(), Vector3Zero(), 0.0f) {}

    DarkBody(Vector3 position, Vector3 velocity, float mass) :
        Body(position, velocity, mass) {}

    void Draw2D() const override {}

    void Draw() const override
    {
#if 0
        DrawCubeV(position, { 2,2,2 }, GREEN);
#endif
    }

    void Randomize() override
    {
        float angle = RandBetween(0.0f, 2.0f * PI);
        float distance = RandBetween(galaxyRadius / 2, galaxyRadius);
        float t = distance / galaxyRadius;
        float eccentricity = RandBetween(-PI / 8, PI / 8) * (1 - t);
        Vector3 startPosition = { 0, distance, 0 };
        Vector3 offsetFromDisc = Vector3RotateByAxisAngle(startPosition, { 1, 0, 0 }, eccentricity);
        Vector3 aroundCenter = Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle);

        position = aroundCenter;
#if 1
        velocity = Vector3RotateByAxisAngle(Vector3Scale(Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle), 0.25f), { 0, 0, 1 }, PI / 3);
#elif 1
        velocity = { RandBetween(-10, 10), RandBetween(-10, 10), RandBetween(-1, 1) };
#else
        velocity = Vector3Zero();
#endif
        mass = Lerp(50.0f, 100.0f, t);
    }
};

int main()
{
    InitWindow(720, 720, "Galaxy Sim");

    SetTargetFPS(0);

    {
        Image img;

        img = GenImageColor(64, 64, WHITE);
        starTexture = LoadTextureFromImage(img);
        UnloadImage(img);

        img = GenImageColor(1, 1, { 255, 255, 255, 127 });
        gasTexture = LoadTextureFromImage(img);
        UnloadImage(img);

        img = GenImagePerlinNoise(32, 32, 0, 0, 20);
        ImageAlphaMask(&img, img);
        dustTexture = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    vector<Body*> bodies;
    bodies.reserve(numBodies);

    srand(2);

    // Black hole
    Star* blackHole = new Star(Vector3Zero(), Vector3Zero(), 4.154e4f, { 0, 0, 0, 255 });
    blackHole->radius = 15.0f;
    bodies.push_back(blackHole);

    for (int i = 0; i < numStars;      ++i) { bodies.push_back(new Star     ()); }
    for (int i = 0; i < numGasClumps;  ++i) { bodies.push_back(new GasClump ()); }
    for (int i = 0; i < numDustClouds; ++i) { bodies.push_back(new DustCloud()); }
    for (int i = 0; i < numDarkBodies; ++i) { bodies.push_back(new DarkBody ()); }

    for (Body* body : bodies)
    {
        if (body != blackHole)
        {
            body->Randomize();
        }
    }

    bool isSimulationPaused = false;

    while (!WindowShouldClose())
    {
#if 1
        //if (isSimulationPaused)
        //{
            UpdateCamera(&camera, CAMERA_ORBITAL);
        //}
        //else
        //{
        //    camera.position = { 0, 0, -galaxyRadius * 2 };
        //}
#endif

        if (IsKeyPressed(KEY_SPACE))
        {
            isSimulationPaused = !isSimulationPaused;
        }

        if (!isSimulationPaused)
        {
            float dt = GetFrameTime() * simulationSpeed;

            auto updateBody = [&bodies, dt](size_t iStart, size_t iEnd) {
                for (size_t i = iStart; i < iEnd; ++i)
                {
                    Body* a = bodies[i];
                    for (size_t j = i; j < numBodies; ++j)
                    {
                        Body* b = bodies[j];

                        float distance = Vector3Distance(a->position, b->position);

                        if (distance < 10.0f)
                        {
                            continue;
                        }

                        float gravity = (G * a->mass * b->mass) / (distance * distance);

                        Vector3 direction = Vector3Normalize(Vector3Subtract(b->position, a->position));
                        Vector3 force = Vector3Scale(direction, gravity);
                        Vector3 accelerationA = Vector3Scale(force, +dt / a->mass);
                        Vector3 accelerationB = Vector3Scale(force, -dt / b->mass);

                        a->velocity = Vector3Add(a->velocity, accelerationA);
                        b->velocity = Vector3Add(b->velocity, accelerationB);
                    }
                }
            };

            // Multithread
            {
                size_t numThreads = std::thread::hardware_concurrency();
                if (numThreads == 0) numThreads = 8;
                std::vector<std::thread> threads;
                threads.reserve(numThreads);

                size_t bodiesPerThread = numBodies / numThreads;
                for (size_t i = 0; i < numThreads; ++i)
                {
                    Body* a = bodies[i];
                    size_t start = i * bodiesPerThread;
                    size_t end = start + bodiesPerThread;
                    threads.push_back(std::thread(updateBody, start, end));
                }

                for (std::thread& thread : threads)
                {
                    thread.join();
                }
            }

            for (Body* body : bodies)
            {
                body->position = Vector3Subtract(Vector3Add(body->position, Vector3Scale(body->velocity, dt)), blackHole->position);
                if (Vector3Length(body->position) > galaxyRadius * 2)
                {
                    body->Randomize();
                }
                
                if (dynamic_cast<DustCloud*>(body) && Vector3Length(body->position) < galaxyRadius / 3)
                {
                    body->Randomize();
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
    UnloadTexture(dustTexture);

    CloseWindow();
}
