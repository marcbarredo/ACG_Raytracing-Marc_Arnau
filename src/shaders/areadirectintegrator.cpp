#include "areadirectintegrator.h"

#include "../core/utils.h"
#include "../lightsources/arealightsource.h"

#include <algorithm>


AreaDirectIntegrator::AreaDirectIntegrator() :
    Shader(Vector3D(0.0))
{
}


AreaDirectIntegrator::AreaDirectIntegrator(Vector3D bgColor_) :
    Shader(bgColor_)
{
}


Vector3D AreaDirectIntegrator::computeColor(const Ray& r, const std::vector<Shape*>& objList, const std::vector<LightSource*>& lsList) const

{
    // MAXIMUM RAY DEPTH
    if (r.depth > 10)
    {
        return bgColor;
    }

    Intersection its;

    if (!Utils::getClosestIntersection(r, objList, its))
    {
        return bgColor;
    }


    const Material& material = its.shape->getMaterial();

    if (material.isEmissive())
    {
        return material.getEmissiveRadiance();
    }

    Vector3D wo = -r.d;

    // PERFECT MIRROR REFLECTION
    if (material.hasSpecular())
    {
        Vector3D wr =
            2.0 * dot(its.normal, wo) * its.normal - wo;

        Ray reflectedRay(
            its.itsPoint,
            wr.normalized(),
            r.depth + 1,
            Epsilon,
            INFINITY
        );

        return computeColor(
            reflectedRay,
            objList,
            lsList
        );
    }


    // PERFECT SPECULAR TRANSMISSION / REFRACTION
    if (material.hasTransmission())
    {
        Vector3D n = its.normal;

        double mu =
            material.getIndexOfRefraction();

        double cosTheta =
            dot(n, wo);


        // If the ray is leaving the object,
        // reverse the normal and invert eta1/eta2
        if (cosTheta < 0.0)
        {
            n = -n;
            cosTheta = -cosTheta;

            mu = 1.0 / mu;
        }


        double radicand = 1.0 - mu * mu * (1.0 - cosTheta * cosTheta);


        // REFRACTION
        if (radicand >= 0.0)
        {
            Vector3D wt =
                -mu * wo +
                n * (
                    mu * cosTheta -
                    std::sqrt(radicand)
                    );

            Ray refractedRay(
                its.itsPoint,
                wt.normalized(),
                r.depth + 1,
                Epsilon,
                INFINITY
            );

            return computeColor(
                refractedRay,
                objList,
                lsList
            );
        }


        // TOTAL INTERNAL REFLECTION
        else
        {
            Vector3D wr =
                2.0 * dot(n, wo) * n - wo;

            Ray reflectedRay(
                its.itsPoint,
                wr.normalized(),
                r.depth + 1,
                Epsilon,
                INFINITY
            );

            return computeColor(
                reflectedRay,
                objList,
                lsList
            );
        }
    }



    // indirect approx:
    // Lo_ind = Ia * kd
    double Ia = 0.3;

    Vector3D color = material.getDiffuseReflectance() * Ia;


    // go through all lights
    for (size_t i = 0; i < lsList.size(); i++)
    {
        AreaLightSource* light = dynamic_cast<AreaLightSource*>(lsList[i]);

        if (light == nullptr)
        {
            continue;
        }


		// y = rand point on light source
        Vector3D y = light->sampleLightPosition();


        // Vector x -> y
        Vector3D toLight = y - its.itsPoint;

        double distance = toLight.length();

        Vector3D wi = toLight.normalized();


        // VISIBILITY V(x,y)

        Ray shadowRay( its.itsPoint, wi, r.depth + 1, Epsilon, distance - Epsilon);

        bool blocked = Utils::hasIntersection( shadowRay, objList);

        if (blocked)
        {
            continue;
        };

        // GEOMETRIC G(x,y)

        double cosSurface = std::max( dot(its.normal, wi), 0.0);

        Vector3D lightNormal = light->getNormal();

        double cosLight = std::max(dot(-wi, lightNormal),0.0);

        double G = (cosSurface * cosLight) / (distance * distance);


        // BRDF
        Vector3D fr =  material.getReflectance( its.normal, wo, wi);

		// Radiance emitted by the light source 
        Vector3D Le = light->getIntensity();

        // uniform pdf
        double pdf = 1.0 / light->getArea();


        // Monte Carlo estimator:
        color +=  Le * fr * G / pdf;
    }


    return color;
}