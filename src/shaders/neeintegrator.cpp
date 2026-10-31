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


// ============================================================
// COMPUTE COLOR
// ============================================================

Vector3D NEEIntegrator::computeColor(
    const Ray &r,
    const std::vector<Shape*> &objList,
    const std::vector<LightSource*> &lsList) const
{
    Intersection its;

    // El ray no toca res
    if (!Utils::getClosestIntersection(r, objList, its))
    {
        return bgColor;
    }


    const Material &material =
        its.shape->getMaterial();


    // ========================================================
    // EMISSIVE MATERIAL
    // ========================================================

    if (material.isEmissive())
    {
        /*
         En NEE no volem comptar Le quan un ray difús
         arriba accidentalment a la llum, perquè aquesta
         contribució directa ja es calcula amb directRadiance().

         Sí que volem Le si:
         1. és el camera ray
         2. venim d'un Mirror o Transmissive
        */

        if (r.depth == 0 || r.specularBounce)
        {
            return material.getEmissiveRadiance();
        }

        return Vector3D(0.0);
    }


    // Límit de rebots
    if (r.depth >= 10)
    {
        return Vector3D(0.0);
    }


    Vector3D wo =
        -r.d;


    return reflectedRadiance(
        its,
        wo,
        r.depth,
        objList,
        lsList
    );
}


// ============================================================
// REFLECTED RADIANCE
//
// Lr = Ldirect + Lindirect
//
// Mirror i Transmissive són casos especials.
// ============================================================

Vector3D NEEIntegrator::reflectedRadiance(
    const Intersection &its,
    const Vector3D &wo,
    int depth,
    const std::vector<Shape*> &objList,
    const std::vector<LightSource*> &lsList) const
{
    const Material &material =
        its.shape->getMaterial();


    // ========================================================
    // PERFECT MIRROR
    // ========================================================

    if (material.hasSpecular())
    {
        Vector3D wr =
            2.0 *
            dot(its.normal, wo) *
            its.normal
            - wo;


        Ray reflectedRay(
            its.itsPoint,
            wr.normalized(),
            depth + 1,
            Epsilon,
            INFINITY
        );


        // El següent ray prové d'un rebot especular
        reflectedRay.specularBounce = true;


        return computeColor(
            reflectedRay,
            objList,
            lsList
        );
    }


    // ========================================================
    // PERFECT TRANSMISSION / REFRACTION
    // ========================================================

    if (material.hasTransmission())
    {
        Vector3D n =
            its.normal;


        double mu =
            material.getIndexOfRefraction();


        double cosTheta =
            dot(n, wo);


        /*
         Si el ray surt de l'objecte:
         - invertim la normal
         - invertim la relació d'índexs
        */
        if (cosTheta < 0.0)
        {
            n = -n;

            cosTheta =
                -cosTheta;

            mu =
                1.0 / mu;
        }


        double radicand =
            1.0 -
            mu * mu *
            (
                1.0 -
                cosTheta * cosTheta
            );


        // ====================================================
        // REFRACTION
        // ====================================================

        if (radicand >= 0.0)
        {
            Vector3D wt =
                -mu * wo +
                n *
                (
                    mu * cosTheta -
                    std::sqrt(radicand)
                );


            Ray refractedRay(
                its.itsPoint,
                wt.normalized(),
                depth + 1,
                Epsilon,
                INFINITY
            );


            // Ray generat per transmissió especular
            refractedRay.specularBounce = true;


            return computeColor(
                refractedRay,
                objList,
                lsList
            );
        }


        // ====================================================
        // TOTAL INTERNAL REFLECTION
        // ====================================================

        else
        {
            Vector3D wr =
                2.0 *
                dot(n, wo) *
                n
                - wo;


            Ray reflectedRay(
                its.itsPoint,
                wr.normalized(),
                depth + 1,
                Epsilon,
                INFINITY
            );


            reflectedRay.specularBounce = true;


            return computeColor(
                reflectedRay,
                objList,
                lsList
            );
        }
    }


    // ========================================================
    // PHONG MATERIAL
    // ========================================================

    Vector3D Ldirect =
        directRadiance(
            its,
            wo,
            objList,
            lsList
        );


    Vector3D Lindirect =
        indirectRadiance(
            its,
            wo,
            depth,
            objList,
            lsList
        );


    return
        Ldirect +
        Lindirect;
}


// ============================================================
// DIRECT RADIANCE
//
// Next Event Estimation:
// sample directament un punt de l'Area Light.
// ============================================================

Vector3D NEEIntegrator::directRadiance(
    const Intersection &its,
    const Vector3D &wo,
    const std::vector<Shape*> &objList,
    const std::vector<LightSource*> &lsList) const
{
    Vector3D Ldirect(0.0);


    const Material &material =
        its.shape->getMaterial();


    for (size_t i = 0;
         i < lsList.size();
         i++)
    {
        AreaLightSource* light =
            dynamic_cast<AreaLightSource*>(
                lsList[i]
            );


        if (light == nullptr)
        {
            continue;
        }


        // Punt aleatori sobre la llum
        Vector3D y =
            light->sampleLightPosition();


        Vector3D toLight =
            y - its.itsPoint;


        double distance =
            toLight.length();


        Vector3D wi =
            toLight.normalized();


        // ====================================================
        // VISIBILITY / SHADOW RAY
        // ====================================================

        Ray shadowRay(
            its.itsPoint,
            wi,
            0,
            Epsilon,
            distance - Epsilon
        );


        bool blocked =
            Utils::hasIntersection(
                shadowRay,
                objList
            );


        if (blocked)
        {
            continue;
        }


        // cos(theta_x)
        double cosSurface =
            std::max(
                dot(
                    its.normal,
                    wi
                ),
                0.0
            );


        // cos(theta_y)
        double cosLight =
            std::max(
                dot(
                    -wi,
                    light->getNormal()
                ),
                0.0
            );


        // Geometry term
        double G =
            cosSurface *
            cosLight /
            (
                distance *
                distance
            );


        // BRDF
        Vector3D fr =
            material.getReflectance(
                its.normal,
                wo,
                wi
            );


        // Radiància emesa per la llum
        Vector3D Le =
            light->getIntensity();


        // Sampling uniforme de l'àrea
        double pdf =
            1.0 /
            light->getArea();


        Ldirect +=
            Le *
            fr *
            G /
            pdf;
    }


    return Ldirect;
}


// ============================================================
// INDIRECT RADIANCE
//
// Sampleig uniforme de l'hemisferi.
// ============================================================

Vector3D NEEIntegrator::indirectRadiance(
    const Intersection &its,
    const Vector3D &wo,
    int depth,
    const std::vector<Shape*> &objList,
    const std::vector<LightSource*> &lsList) const
{
    Vector3D Lindirect(0.0);


    // Límit de profunditat
    if (depth + 1 >= 10)
    {
        return Lindirect;
    }


    const Material &material =
        its.shape->getMaterial();


    // ========================================================
    // SAMPLE HEMISPHERE
    // ========================================================

    HemisphericalSampler sampler;


    Vector3D wi =
        sampler
            .getSample(its.normal)
            .normalized();


    // PDF uniforme hemisferi
    double pdf =
        1.0 /
        (2.0 * 3.1416);


    // Ray indirecte Phong
    Ray newRay(
        its.itsPoint,
        wi,
        depth + 1,
        Epsilon,
        INFINITY
    );


    // Aquest ray NO prové d'un rebot especular
    newRay.specularBounce = false;


    Intersection nextIts;


    // Si no toca res -> 0
    if (!Utils::getClosestIntersection(
            newRay,
            objList,
            nextIts))
    {
        return Lindirect;
    }


    const Material &nextMaterial =
        nextIts.shape->getMaterial();


    /*
     IMPORTANT:

     Si el ray aleatori Phong toca directament la llum,
     NO sumem Le.

     Aquesta contribució ja ha estat calculada per
     directRadiance() mitjançant NEE.
    */
    if (nextMaterial.isEmissive())
    {
        return Lindirect;
    }


    Vector3D nextWo =
        -wi;


    /*
     Aquí cridem reflectedRadiance().

     Si nextMaterial és:
       - Phong -> continua amb NEE normal
       - Mirror -> genera reflected ray
       - Transmissive -> genera refracted ray

     Això permet paths com:

         Phong -> Mirror -> Light

     o:

         Phong -> Transmissive -> Light
    */

    Vector3D Li =
        reflectedRadiance(
            nextIts,
            nextWo,
            depth + 1,
            objList,
            lsList
        );


    // BRDF de la superfície actual
    Vector3D fr =
        material.getReflectance(
            its.normal,
            wo,
            wi
        );


    // cos(theta)
    double cosTheta =
        std::max(
            dot(
                its.normal,
                wi
            ),
            0.0
        );


    // Monte Carlo estimator
    Lindirect =
        Li *
        fr *
        cosTheta /
        pdf;


    return Lindirect;
}
