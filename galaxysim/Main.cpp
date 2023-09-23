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
constexpr size_t numStars = 8192;
constexpr size_t numGasClumps = 512;
constexpr size_t numDustClouds = 512;
constexpr size_t numDarkBodies = 1025;
constexpr size_t numBodies = 1 + numStars + numGasClumps + numDustClouds + numDarkBodies;
constexpr float simulationSpeed = 1.0f;
Camera camera = { { 0, 0, -galaxyRadius * 1.5f }, Vector3Zero(), {0, 1, 0}, 120.0f, CAMERA_PERSPECTIVE};

constexpr float G = 6;

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
#if 0 // Show velocity
        DrawLine3D(position, Vector3Add(position, Vector3Scale(velocity, GetFrameTime() * simulationSpeed)), { 0, 127, 0, 64 });
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
        velocity = Vector3RotateByAxisAngle(Vector3Scale(Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle), 0.25f), { 0, 0, 1 }, PI / 3);
        mass = Lerp(8.0f, 0.5f, t);
        radius = StarRadius(mass);
        color = ColorLerp(ColorLerp(amyblue, WHITE, t), ColorLerp(WHITE, amypurple, t), t);

        if (RandBetween(0, 1) < 0.01f)
        {
            color = WHITE;
        }
        if (RandBetween(0, 1) < 0.0001f)
        {
            color = RED;
        }
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
        // additive blending makes the blue turn green :c
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
        velocity = Vector3RotateByAxisAngle(Vector3Scale(Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle), 0.25f), { 0, 0, 1 }, PI / 3);
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
        float t = Clamp(Vector3Length(position) / galaxyRadius, 0.0f, 1.0f);
        DrawBillboard(camera, dustTexture, position, mass * 32, ColorLerp(amyfur, amyskin, t));
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
        velocity = Vector3RotateByAxisAngle(Vector3Scale(Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle), 0.25f), { 0, 0, 1 }, PI / 3);
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
#if 0 // Show dark matter
        DrawSphere(position, mass, { 0, 127, 0, 63 });
#endif
#if 0 // Show dark matter velocity
        DrawLine3D(position, Vector3Add(position, velocity), RED);
#endif
    }

    void Randomize() override
    {
        float angle = RandBetween(0.0f, 2.0f * PI);
        float distance = RandBetween(galaxyRadius / 2, galaxyRadius * 1.5f);
        float t = distance / galaxyRadius;
        float eccentricity = RandBetween(-PI / 8, PI / 8) * (1 - t);
        Vector3 startPosition = { 0, distance, 0 };
        Vector3 offsetFromDisc = Vector3RotateByAxisAngle(startPosition, { 1, 0, 0 }, eccentricity);
        Vector3 aroundCenter = Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle);

        position = aroundCenter;
        velocity = Vector3RotateByAxisAngle(Vector3Scale(Vector3RotateByAxisAngle(offsetFromDisc, { 0, 0, 1 }, angle), 0.25f), { 0, 0, 1 }, PI / 3);
        mass = Lerp(20, 30.0f, t);
    }
};

int main()
{
    int windowWidth = 1280;
    int windowHeight = 720;
    InitWindow(windowWidth, windowHeight, "Galaxy Sim");

    SetTargetFPS(0);

    Texture background;

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

        img = GenImagePerlinNoise(windowWidth, windowWidth, 0, 0, 10);
        background = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    vector<Body*> bodies;
    bodies.reserve(numBodies);

    //srand(2);

    // Black hole
    Star* blackHole = new Star(Vector3Zero(), Vector3Zero(), 4154, { 0, 0, 0, 255 });
    blackHole->radius = 15.0f; // black hole is tiny despite mass
    bodies.push_back(blackHole);

    for (int i = 0; i < numStars;      ++i) { bodies.push_back(new Star     ()); }
    for (int i = 0; i < numGasClumps;  ++i) { bodies.push_back(new GasClump ()); }
    for (int i = 0; i < numDustClouds; ++i) { bodies.push_back(new DustCloud()); }
    for (int i = 0; i < numDarkBodies; ++i) { bodies.push_back(new DarkBody ()); }

    // Skips black hole
    for (size_t i = 1; i < numBodies; ++i)
    {
        bodies[i]->Randomize();
    }

    bool isSimulationPaused = false;
    enum class View { Orbit, Front, Side, Star } view = View::Front;
    size_t observedStar = 0;

    while (!WindowShouldClose())
    {
        switch (view)
        {
        case View::Front:
            camera.position = { 0, 0, galaxyRadius * -1.5f };
            camera.up = { 0, 1, 0 };
            camera.fovy = 120;
            break;
        case View::Side:
            camera.position = { galaxyRadius * -1.5f, 0, 0 };
            camera.up = { 0, 1, 0 };
            camera.fovy = 120;
            break;
        case View::Star:
        {
            Body* body = bodies[observedStar];
            camera.position = body->position;
            camera.up = Vector3RotateByAxisAngle(body->position, { 0, 0, 1, }, PI / 2);
            camera.fovy = 45;
        }
            break;

        default:
        case View::Orbit:
            UpdateCamera(&camera, CAMERA_ORBITAL);
            camera.up = { 0, 1, 0 };
            camera.fovy = 120;
            break;
        }

        if (IsKeyPressed(KEY_SPACE))
        {
            isSimulationPaused = !isSimulationPaused;
        }

        if (IsKeyPressed(KEY_F)) view = View::Front;
        if (IsKeyPressed(KEY_S)) view = View::Side;
        if (IsKeyPressed(KEY_O)) view = View::Orbit;
        if (IsKeyPressed(KEY_UP))   { view = View::Star; ++observedStar %= numStars; }
        if (IsKeyPressed(KEY_DOWN)) { view = View::Star; --observedStar %= numStars; }

        if (!isSimulationPaused)
        {
            float dt = GetFrameTime() * simulationSpeed;

            auto updateBody = [&](size_t iStart, size_t iEnd)
            {
                for (size_t i = iStart; i < iEnd; ++i)
                {
                    Body* a = bodies[i];

                    // Skip black holes and darkmatter
                    if (a == blackHole || dynamic_cast<DarkBody*>(a))
                    {
                        continue;
                    }
                    else if (a != nullptr)
                    {
                        for (Body* b : bodies)
                        {
                            float distance = Vector3Distance(a->position, b->position);

                            // Skip (prbably) colliding
                            if (distance < 1.0f)
                            {
                                continue;
                            }

                            float gravity = (G * a->mass * b->mass) / (distance * distance);

                            Vector3 direction = Vector3Normalize(Vector3Subtract(b->position, a->position));
                            Vector3 force = Vector3Scale(direction, gravity);
                            Vector3 acceleration = Vector3Scale(force, +dt / a->mass);

                            a->velocity = Vector3Add(a->velocity, acceleration);
                        }
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

            // Skips black hole
            for (size_t i = 1; i < numBodies; ++i)
            {
                Body* body = bodies[i];

                if (DarkBody* darkmatter = dynamic_cast<DarkBody*>(body))
                {
                    float t = Vector3Length(darkmatter->position) / (galaxyRadius * 1.5f);
                    Vector3 newPosition = Vector3RotateByAxisAngle(darkmatter->position, { 0, 0, 1 }, (1 - powf(t, 2.0f)) * dt * PI / 8);
                    darkmatter->velocity = Vector3Subtract(newPosition, darkmatter->position); // Makes the debug look better
                    darkmatter->position = newPosition;
                    continue;
                }
                else if (body != nullptr) // I hate that I have to put this here for the compiler's sake.
                {
                    body->position = Vector3Add(body->position, Vector3Scale(body->velocity, dt));

                    // Bodies beyond the galaxy get culled and reused
                    if (Vector3Length(body->position) > galaxyRadius * 2)
                    {
                        body->Randomize();
                    }

                    // Dust clouds burn up in the galactic core
                    if (dynamic_cast<DustCloud*>(body) && Vector3Length(body->position) < galaxyRadius / 3)
                    {
                        body->Randomize();
                    }
                }
            }
        }

        BeginDrawing();

            ClearBackground(BLACK);

            DrawTexture(background, 0, 0, ColorLerp(BLACK, amypurple, 0.1f));

            // Workaround for far plane cull
            // Still no fix when the body simply doesn't look right as a point (like gas or dust)
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

#if 0 // black hole crosshair
            //if (view == View::Star)
            {
                DrawRing(Vector2Add(GetWorldToScreen(Vector3Zero(), camera), { 1,1 }), 2, 4, 0, 360, 36, BLACK);
                DrawRing(GetWorldToScreen(Vector3Zero(), camera), 2, 4, 0, 360, 36, RED);
            }
#endif

            DrawFPS(0,0);

            if (isSimulationPaused)
            {
                DrawText("PAUSED", 0, 30, 8, WHITE);
            }

            if (observedStar == 0 && view == View::Star)
            {
                DrawText("WARNING: You are inside a black hole.", 0, 40, 8, YELLOW);
            }

        EndDrawing();
    }

    UnloadTexture(starTexture);
    UnloadTexture(gasTexture);
    UnloadTexture(dustTexture);
    UnloadTexture(background);

    CloseWindow();
}
