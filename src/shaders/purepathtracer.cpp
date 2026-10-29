#include "purepathtracer.h"

#include "../core/utils.h"
#include "../core/hemisphericalsampler.h"

#include <algorithm>
#include <cmath>


PurePathTracer::PurePathTracer() :
    Shader(Vector3D(0.0))
{
}


PurePathTracer::PurePathTracer(Vector3D bgColor_) :
    Shader(bgColor_)
{
}


Vector3D PurePathTracer::computeColor(
    const Ray &r,
    const std::vector<Shape*> &objList,
    const std::vector<LightSource*> &lsList) const
{
    Intersection its;

    // Si el ray no toca res -> fons
    if (!Utils::getClosestIntersection(r, objList, its))
    {
        return bgColor;
    }


    const Material &material =
        its.shape->getMaterial();


    // Radiància emesa per la superfície
    // Per defecte és 0 si el material no és emissiu
    Vector3D Lo(0.0);

    if (material.isEmissive())
    {
        Lo = material.getEmissiveRadiance();
    }


    // Límit de profunditat del path
    if (r.depth >= 10)
    {
        return Lo;
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


    // Mostregem una direcció aleatòria de l'hemisferi
    HemisphericalSampler sampler;

    Vector3D wi =
        sampler.getSample(its.normal).normalized();


    // PDF uniforme sobre l'hemisferi
    double pdf =
        1.0 / (2.0 * 3.1416);


    // Nou ray per continuar el path
    Ray newRay(
        its.itsPoint,
        wi,
        r.depth + 1,
        Epsilon,
        INFINITY
    );


    // BRDF del material actual
    Vector3D fr =
        material.getReflectance(
            its.normal,
            wo,
            wi
        );


    // Terme cosinus de la rendering equation
    double cosTheta =
        std::max(
            dot(its.normal, wi),
            0.0
        );


    // Radiància que arriba des del següent rebot
    Vector3D Li =
        computeColor(
            newRay,
            objList,
            lsList
        );


    // Estimador Monte Carlo de la rendering equation
    Lo +=
        Li * fr * cosTheta / pdf;


    return Lo;
}
