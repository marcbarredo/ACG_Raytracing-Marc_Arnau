// UPF - ACG - 2026_27

#include <iostream>
#include <algorithm>
#include <chrono>

// Core
#include "core/film.h"
#include "core/matrix4x4.h"
#include "core/ray.h"
#include "core/utils.h"
#include "core/scene.h"

// Shapes
#include "shapes/sphere.h"
#include "shapes/infiniteplan.h"
#include "shapes/square.h"

// Cameras
#include "cameras/ortographic.h"
#include "cameras/perspective.h"

// Shaders
#include "shaders/intersectionshader.h"
#include "shaders/depthshader.h"
#include "shaders/normalshader.h"
#include "shaders/whittedintegrator.h"
#include "shaders/hemisphericaldirectintegrator.h"
#include "shaders/areadirectintegrator.h"
#include "shaders/purepathtracer.h"
#include "shaders/neeintegrator.h"

// Lights
#include "lightsources/arealightsource.h"

// Materials
#include "materials/phong.h"
#include "materials/emissive.h"
#include "materials/mirror.h"
#include "materials/transmissive.h"


using namespace std::chrono;

typedef std::chrono::duration<double, std::milli> durationMs;


// ============================================================
// CORNELL BOX
// ============================================================

void buildSceneCornellBox(
    Camera*& cam,
    Film*& film,
    Scene myScene)
{
    // ========================================================
    // CAMERA
    // ========================================================

    Matrix4x4 cameraToWorld =
        Matrix4x4::translate(
            Vector3D(0.0, 0.0, -3.0)
        );


    double fovDegrees =
        60.0;


    double fovRadians =
        Utils::degreesToRadians(
            fovDegrees
        );


    cam =
        new PerspectiveCamera(
            cameraToWorld,
            fovRadians,
            *film
        );


    // ========================================================
    // MATERIALS
    // ========================================================

    Material* redDiffuse =
        new Phong(
            Vector3D(0.7, 0.2, 0.3),
            Vector3D(0.0),
            100
        );


    Material* greenDiffuse =
        new Phong(
            Vector3D(0.2, 0.7, 0.3),
            Vector3D(0.0),
            100
        );


    Material* greyDiffuse =
        new Phong(
            Vector3D(0.8, 0.8, 0.8),
            Vector3D(0.0),
            100
        );


    Material* blueGlossy_20 =
        new Phong(
            Vector3D(0.2, 0.3, 0.5),
            Vector3D(0.8, 0.7, 0.5),
            20
        );


    Material* blueGlossy_80 =
        new Phong(
            Vector3D(0.2, 0.3, 0.5),
            Vector3D(0.8, 0.7, 0.5),
            80
        );


    Material* cyanDiffuse =
        new Phong(
            Vector3D(0.2, 0.8, 0.8),
            Vector3D(0.0),
            100
        );


    // Area Light
    Material* emissive =
        new Emissive(
            Vector3D(25.0, 25.0, 25.0),
            Vector3D(0.5)
        );


    // 4.3.3
    Material* mirror =
        new Mirror();


    Material* transmissive =
        new Transmissive(
            0.7
        );


    // ========================================================
    // CORNELL BOX
    // ========================================================

    double offset =
        3.0;


    Shape* leftPlan =
        new InfinitePlan(
            Vector3D(
                -offset - 1.0,
                0.0,
                0.0
            ),
            Vector3D(
                1.0,
                0.0,
                0.0
            ),
            redDiffuse
        );


    Shape* rightPlan =
        new InfinitePlan(
            Vector3D(
                offset + 1.0,
                0.0,
                0.0
            ),
            Vector3D(
                -1.0,
                0.0,
                0.0
            ),
            greenDiffuse
        );


    Shape* topPlan =
        new InfinitePlan(
            Vector3D(
                0.0,
                offset,
                0.0
            ),
            Vector3D(
                0.0,
                -1.0,
                0.0
            ),
            greyDiffuse
        );


    Shape* bottomPlan =
        new InfinitePlan(
            Vector3D(
                0.0,
                -offset,
                0.0
            ),
            Vector3D(
                0.0,
                1.0,
                0.0
            ),
            greyDiffuse
        );


    Shape* backPlan =
        new InfinitePlan(
            Vector3D(
                0.0,
                0.0,
                3.0 * offset
            ),
            Vector3D(
                0.0,
                0.0,
                -1.0
            ),
            greyDiffuse
        );


    // ========================================================
    // AREA LIGHT
    // ========================================================

    Square* square_emissive =
        new Square(
            Vector3D(
                -1.0,
                3.0,
                3.0
            ),
            Vector3D(
                2.0,
                0.0,
                0.0
            ),
            Vector3D(
                0.0,
                0.0,
                2.0
            ),
            Vector3D(
                0.0,
                -1.0,
                0.0
            ),
            emissive
        );


    // Add Cornell Box
    myScene.AddObject(leftPlan);
    myScene.AddObject(rightPlan);
    myScene.AddObject(topPlan);
    myScene.AddObject(bottomPlan);
    myScene.AddObject(backPlan);

    /*
     IMPORTANT:
     En el vostre framework AddObject() ja detecta
     els materials emissius i els afegeix com AreaLight.

     NO fem un push_back manual a LightSourceList,
     perquè duplicaríem la llum.
    */
    myScene.AddObject(square_emissive);


    // ========================================================
    // OBJECTS
    // ========================================================

    double radius =
        1.0;


    // --------------------------------------------------------
    // PHONG SPHERE
    // --------------------------------------------------------

    Matrix4x4 sphereTransform1;


    sphereTransform1 =
        Matrix4x4::translate(
            Vector3D(
                1.5,
                -offset + radius,
                6.0
            )
        );


    Shape* s1 =
        new Sphere(
            radius,
            sphereTransform1,
            blueGlossy_20
        );


    // --------------------------------------------------------
    // TRANSMISSIVE SPHERE
    // --------------------------------------------------------

    Matrix4x4 sphereTransform2;


    sphereTransform2 =
        Matrix4x4::translate(
            Vector3D(
                -1.5,
                -offset + 3.0 * radius,
                4.0
            )
        );


    Shape* s2 =
        new Sphere(
            radius,
            sphereTransform2,
            transmissive
        );


    // --------------------------------------------------------
    // MIRROR SQUARE
    // --------------------------------------------------------

    Shape* square =
        new Square(
            Vector3D(
                offset + 0.999,
                -offset - 0.2,
                3.0
            ),

            Vector3D(
                0.0,
                4.0,
                0.0
            ),

            Vector3D(
                0.0,
                0.0,
                2.0
            ),

            Vector3D(
                -1.0,
                0.0,
                0.0
            ),

            mirror
        );


    // Add objects
    myScene.AddObject(s1);
    myScene.AddObject(s2);
    myScene.AddObject(square);
}


// ============================================================
// RAYTRACE
// ============================================================

void raytrace(
    Camera*& cam,
    Shader*& shader,
    Film*& film,
    std::vector<Shape*>*& objectsList,
    std::vector<LightSource*>*& lightSourceList)
{
    size_t resX =
        film->getWidth();


    size_t resY =
        film->getHeight();


    // ========================================================
    // SAMPLES PER PIXEL
    // ========================================================

    const int spp =
    1024;


    // ========================================================
    // RENDER LOOP
    // ========================================================

    for (size_t lin = 0;
         lin < resY;
         lin++)
    {
        double progress =
            (double)lin /
            (double)resY;


        Utils::printProgress(
            progress
        );


        for (size_t col = 0;
             col < resX;
             col++)
        {
            // Pixel position in NDC
            double x =
                (double)(col + 0.5) /
                (double)resX;


            double y =
                (double)(lin + 0.5) /
                (double)resY;


            Vector3D pixelColor(
                0.0
            );


            // =================================================
            // MONTE CARLO SAMPLES
            // =================================================

            for (int s = 0;
                 s < spp;
                 s++)
            {
                Ray cameraRay =
                    cam->generateRay(
                        x,
                        y
                    );


                pixelColor +=
                    shader->computeColor(
                        cameraRay,
                        *objectsList,
                        *lightSourceList
                    );
            }


            // Average samples
            pixelColor /=
                (double)spp;


            // Store pixel
            film->setPixelValue(
                col,
                lin,
                pixelColor
            );
        }
    }
}


// ============================================================
// TASK 1
// ============================================================

void PaintImage(
    Film* film)
{
    unsigned int sizeBar =
        40;


    size_t resX =
        film->getWidth();


    size_t resY =
        film->getHeight();


    for (size_t lin = 0;
         lin < resY;
         lin++)
    {
        if (
            lin %
            (resY / sizeBar)
            == 0
        )
        {
            std::cout
                << ".";
        }


        for (size_t col = 0;
             col < resX;
             col++)
        {
            Vector3D color(
                (double)col /
                resX,

                (double)lin /
                resY,

                0.0
            );


            film->setPixelValue(
                col,
                lin,
                color
            );
        }
    }
}


// ============================================================
// MAIN
// ============================================================

int main()
{
    std::string separator =
        "\n----------------------------------------------\n";


    std::cout
        << separator
        << "RT-ACG - Ray Tracer for \"Advanced Computer Graphics\""
        << separator
        << std::endl;


    // ========================================================
    // FILM
    // ========================================================

    Film* film =
        new Film(
            720,
            512
        );


    // ========================================================
    // BACKGROUND
    // ========================================================

    Vector3D bgColor(
        0.0,
        0.0,
        0.0
    );


    // ========================================================
    // SHADERS / INTEGRATORS
    // ========================================================

    // Lab 1
    // Shader* whittedShader =
    //     new WhittedIntegrator(bgColor);


    // Lab 2 - 4.2.1
    // Shader* hemisphericalDirectShader =
    //     new HemisphericalDirectIntegrator(bgColor);


    // Lab 2 - 4.2.2
    // Shader* areaDirectShader =
    //     new AreaDirectIntegrator(bgColor);


    // Lab 2 - 4.3.1
    // Shader* purePathShader =
    //     new PurePathTracer(bgColor);


    // ========================================================
    // Lab 2 - 4.3.2 + 4.3.3
    // NEE + Mirror + Transmissive
    // ========================================================

    Shader* neeShader =
        new NEEIntegrator(
            bgColor
        );


    // ========================================================
    // SCENE
    // ========================================================

    Camera* cam;


    Scene myScene;


    buildSceneCornellBox(
        cam,
        film,
        myScene
    );


    // ========================================================
    // RENDER
    // ========================================================

    auto start =
        high_resolution_clock::now();


    raytrace(
        cam,
        neeShader,
        film,
        myScene.objectsList,
        myScene.LightSourceList
    );


    auto stop =
        high_resolution_clock::now();


    // ========================================================
    // SAVE
    // ========================================================

    std::cout
        << "\n\nSaving the result to file output.bmp\n"
        << std::endl;


    film->save();


    film->saveEXR();


    float durationS =
        (
            durationMs(
                stop -
                start
            )
            /
            1000.0
        ).count();


    std::cout
        << "FINAL_TIME(s): "
        << durationS
        << std::endl;


    std::cout
        << "\n\n"
        << std::endl;


    return 0;
}
