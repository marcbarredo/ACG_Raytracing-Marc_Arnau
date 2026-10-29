#include "neeintegrator.h"

#include "../core/utils.h"
#include "../core/hemisphericalsampler.h"
#include "../lightsources/arealightsource.h"

#include <algorithm>
#include <cmath>


NEEIntegrator::NEEIntegrator() :
    Shader(Vector3D(0.0))
{
}


NEEIntegrator::NEEIntegrator(Vector3D bgColor_) :
    Shader(bgColor_)
{
}


Vector3D NEEIntegrator::computeColor(
    const Ray &r,
    const std::vector<Shape*> &objList,
    const std::vector<LightSource*> &lsList) const
{
    Intersection its;

    // No hit -> background
    if (!Utils::getClosestIntersection(r, objList, its))
    {
        return bgColor;
    }

    const Material &material =
        its.shape->getMaterial();


    // Emitted radiance
    Vector3D Le(0.0);

    if (material.isEmissive())
    {
        Le = material.getEmissiveRadiance();
    }


    // Maximum path depth
    if (r.depth >= 10)
    {
        return Le;
    }


    // Task 4.3.2: només Phong
    if (material.hasSpecular() ||
        material.hasTransmission())
    {
        return Le;
    }


    Vector3D wo = -r.d;

    Vector3D Ldirect(0.0);
    Vector3D Lindirect(0.0);


    // =========================================
    // DIRECT ILLUMINATION - AREA LIGHT SAMPLING
    // =========================================

    for (size_t i = 0; i < lsList.size(); i++)
    {
        AreaLightSource* light =
            dynamic_cast<AreaLightSource*>(lsList[i]);

        if (light == nullptr)
        {
            continue;
        }


        // Punt aleatori y sobre la llum
        Vector3D y =
            light->sampleLightPosition();


        Vector3D toLight =
            y - its.itsPoint;

        double distance =
            toLight.length();

        Vector3D wi =
            toLight.normalized();


        // Shadow ray -> V(x,y)
        Ray shadowRay(
            its.itsPoint,
            wi,
            r.depth + 1,
            Epsilon,
            distance - Epsilon
        );


        bool blocked =
            Utils::hasIntersection(
                shadowRay,
                objList
            );


        if (!blocked)
        {
            double cosSurface =
                std::max(
                    dot(its.normal, wi),
                    0.0
                );

            double cosLight =
                std::max(
                    dot(-wi, light->getNormal()),
                    0.0
                );


            double G =
                (cosSurface * cosLight) /
                (distance * distance);


            Vector3D fr =
                material.getReflectance(
                    its.normal,
                    wo,
                    wi
                );


            Vector3D LeLight =
                light->getIntensity();


            // Uniform area sampling
            double pdfArea =
                1.0 / light->getArea();


            Ldirect +=
                LeLight * fr * G / pdfArea;
        }
    }


    // =========================================
    // INDIRECT ILLUMINATION - HEMISPHERE SAMPLE
    // =========================================

    HemisphericalSampler sampler;

    Vector3D wi =
        sampler.getSample(its.normal).normalized();


    double pdfHemisphere =
        1.0 / (2.0 * 3.1416);


    Ray indirectRay(
        its.itsPoint,
        wi,
        r.depth + 1,
        Epsilon,
        INFINITY
    );


    Vector3D fr =
        material.getReflectance(
            its.normal,
            wo,
            wi
        );


    double cosTheta =
        std::max(
            dot(its.normal, wi),
            0.0
        );


    Vector3D Li =
        computeColor(
            indirectRay,
            objList,
            lsList
        );


    Lindirect =
        Li * fr * cosTheta / pdfHemisphere;


    // Rendering equation:
    // Lo = Le + Ldirect + Lindirect
    return Le + Ldirect + Lindirect;
}
