#pragma once
#include <cmath>

namespace Core::Math {

    constexpr float PI = 3.14159265358979323846f;
    constexpr float DEG_TO_RAD = PI / 180.0f;

    struct Vector2 {
        float X = 0.0f;
        float Y = 0.0f;

        Vector2() = default;
        constexpr Vector2(float x, float y) : X(x), Y(y) {}
    };

    struct Vector3 {
        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;

        Vector3() = default;
        constexpr Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}

        constexpr Vector3 operator+(const Vector3& o) const { return Vector3(X + o.X, Y + o.Y, Z + o.Z); }
        constexpr Vector3 operator-(const Vector3& o) const { return Vector3(X - o.X, Y - o.Y, Z - o.Z); }
        constexpr Vector3 operator*(float s) const { return Vector3(X * s, Y * s, Z * s); }
        constexpr Vector3 operator/(float s) const { return Vector3(X / s, Y / s, Z / s); }

        Vector3& operator+=(const Vector3& o) {
            X += o.X; Y += o.Y; Z += o.Z;
            return *this;
        }

        Vector3& operator-=(const Vector3& o) {
            X -= o.X; Y -= o.Y; Z -= o.Z;
            return *this;
        }

        float Length() const {
            return std::sqrt(X * X + Y * Y + Z * Z);
        }

        float DistTo(const Vector3& o) const {
            float dx = X - o.X;
            float dy = Y - o.Y;
            float dz = Z - o.Z;
            return std::sqrt(dx * dx + dy * dy + dz * dz);
        }

        bool IsZero(float tolerance = 0.001f) const {
            return std::fabs(X) < tolerance && std::fabs(Y) < tolerance && std::fabs(Z) < tolerance;
        }
    };

    struct Rotator {
        float Pitch = 0.0f;
        float Yaw = 0.0f;
        float Roll = 0.0f;

        Rotator() = default;
        constexpr Rotator(float pitch, float yaw, float roll) : Pitch(pitch), Yaw(yaw), Roll(roll) {}
    };

    inline void GetAxes(const Rotator& rot, Vector3& outForward, Vector3& outRight, Vector3& outUp) {
        const float p = rot.Pitch * DEG_TO_RAD;
        const float y = rot.Yaw * DEG_TO_RAD;
        const float r = rot.Roll * DEG_TO_RAD;

        const float sp = std::sin(p), cp = std::cos(p);
        const float sy = std::sin(y), cy = std::cos(y);
        const float sr = std::sin(r), cr = std::cos(r);

        outForward = Vector3(cp * cy, cp * sy, sp);
        outRight   = Vector3(sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp);
        outUp      = Vector3(-(cr * sp * cy + sr * sy), -cr * sp * sy + sr * cy, cr * cp);
    }

    inline bool WorldToScreen(
        const Vector3& worldLoc,
        const Vector3& camLoc,
        const Rotator& camRot,
        float fov,
        float screenWidth,
        float screenHeight,
        Vector2& outScreen
    ) {
        Vector3 axisX, axisY, axisZ;
        GetAxes(camRot, axisX, axisY, axisZ);

        const Vector3 delta = worldLoc - camLoc;
        const Vector3 transformed(
            delta.X * axisY.X + delta.Y * axisY.Y + delta.Z * axisY.Z,
            delta.X * axisZ.X + delta.Y * axisZ.Y + delta.Z * axisZ.Z,
            delta.X * axisX.X + delta.Y * axisX.Y + delta.Z * axisX.Z
        );

        if (transformed.Z < 1.0f)
            return false;

        const float fovRad = (fov * 0.5f) * DEG_TO_RAD;
        const float screenCenterX = screenWidth * 0.5f;
        const float screenCenterY = screenHeight * 0.5f;

        const float fovScale = screenCenterX / std::tan(fovRad);
        outScreen.X = screenCenterX + transformed.X * fovScale / transformed.Z;
        outScreen.Y = screenCenterY - transformed.Y * fovScale / transformed.Z;

        return (outScreen.X >= 0.0f && outScreen.X <= screenWidth &&
                outScreen.Y >= 0.0f && outScreen.Y <= screenHeight);
    }

}
