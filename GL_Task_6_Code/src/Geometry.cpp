/*
 * Copyright 2023 Vienna University of Technology.
 * Institute of Computer Graphics and Algorithms.
 * This file is part of the GCG Lab Framework and must not be redistributed.
*/
/*
#include "Geometry.h"

#undef min
#undef max*/
/*
Geometry::Geometry(glm::mat4 modelMatrix, const GeometryData& data, std::shared_ptr<Material> material)
    : elements{static_cast<unsigned int>(data.indices.size())}
    , modelMatrix{modelMatrix}
    , material{material} {
    // create VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // create positions VBO
    glGenBuffers(1, &vboPositions);
    glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
    glBufferData(GL_ARRAY_BUFFER, data.positions.size() * sizeof(glm::vec3), data.positions.data(), GL_STATIC_DRAW);

    // bind positions to location 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // create normals VBO
    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, data.normals.size() * sizeof(glm::vec3), data.normals.data(), GL_STATIC_DRAW);

    // bind normals to location 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // create uvs VBO
    glGenBuffers(1, &vboUVs);
    glBindBuffer(GL_ARRAY_BUFFER, vboUVs);
    glBufferData(GL_ARRAY_BUFFER, data.uvs.size() * sizeof(glm::vec2), data.uvs.data(), GL_STATIC_DRAW);

    // bind uvs to location 2
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    if (data.colors.size() > 0) {
        glGenBuffers(1, &vboColor);
        glBindBuffer(GL_ARRAY_BUFFER, vboColor);
        glBufferData(GL_ARRAY_BUFFER, data.colors.size() * sizeof(glm::vec3), data.colors.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    // create and bind indices VBO
    glGenBuffers(1, &vboIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Geometry::~Geometry() {
    glDeleteBuffers(1, &vboPositions);
    glDeleteBuffers(1, &vboNormals);
    glDeleteBuffers(1, &vboUVs);
    glDeleteBuffers(1, &vboIndices);
    glDeleteVertexArrays(1, &vao);
}

void Geometry::draw() {
    Shader* shader = material->getShader();
    shader->use();

    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(modelMatrix))));
    material->setUniforms();

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Geometry::transform(glm::mat4 transformation) { modelMatrix = transformation * modelMatrix; }

void Geometry::resetModelMatrix() { modelMatrix = glm::mat4(1); }

GeometryData Geometry::createCubeGeometry(float width, float height, float depth) {
    GeometryData data;

    data.positions = {
        // front
        glm::vec3(-width / 2.0f, -height / 2.0f, depth / 2.0f),
        glm::vec3(width / 2.0f, -height / 2.0f, depth / 2.0f),
        glm::vec3(width / 2.0f, height / 2.0f, depth / 2.0f),
        glm::vec3(-width / 2.0f, height / 2.0f, depth / 2.0f),
        // back
        glm::vec3(width / 2.0f, -height / 2.0f, -depth / 2.0f),
        glm::vec3(-width / 2.0f, -height / 2.0f, -depth / 2.0f),
        glm::vec3(-width / 2.0f, height / 2.0f, -depth / 2.0f),
            glm::vec3(width / 2.0f, height / 2.0f, -depth / 2.0f),
        // right
        glm::vec3(width / 2.0f, -height / 2.0f, depth / 2.0f),
        glm::vec3(width / 2.0f, -height / 2.0f, -depth / 2.0f),
        glm::vec3(width / 2.0f, height / 2.0f, -depth / 2.0f),
        glm::vec3(width / 2.0f, height / 2.0f, depth / 2.0f),
        // left
        glm::vec3(-width / 2.0f, -height / 2.0f, -depth / 2.0f),
        glm::vec3(-width / 2.0f, -height / 2.0f, depth / 2.0f),
        glm::vec3(-width / 2.0f, height / 2.0f, depth / 2.0f),
        glm::vec3(-width / 2.0f, height / 2.0f, -depth / 2.0f),
        // top
        glm::vec3(-width / 2.0f, height / 2.0f, -depth / 2.0f),
        glm::vec3(-width / 2.0f, height / 2.0f, depth / 2.0f),
        glm::vec3(width / 2.0f, height / 2.0f, depth / 2.0f),
        glm::vec3(width / 2.0f, height / 2.0f, -depth / 2.0f),
        // bottom
        glm::vec3(-width / 2.0f, -height / 2.0f, -depth / 2.0f),
        glm::vec3(width / 2.0f, -height / 2.0f, -depth / 2.0f),
        glm::vec3(width / 2.0f, -height / 2.0f, depth / 2.0f),
        glm::vec3(-width / 2.0f, -height / 2.0f, depth / 2.0f)
    };

// clang-format off
	data.normals = {
		// front
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		// back
		glm::vec3(0, 0, -1),
		glm::vec3(0, 0, -1),
		glm::vec3(0, 0, -1),
		glm::vec3(0, 0, -1),
		// right
		glm::vec3(1, 0, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(1, 0, 0),
		// left
		glm::vec3(-1, 0, 0),
		glm::vec3(-1, 0, 0),
		glm::vec3(-1, 0, 0),
		glm::vec3(-1, 0, 0),
		// top
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0),
		// bottom
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0)
	};

	data.uvs = {
		// front
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		glm::vec2(0, 1),
		// back
		glm::vec2(1, 1),
		glm::vec2(0, 1),
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		// right
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		glm::vec2(0, 1),
		// left
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		glm::vec2(0, 1),
		// top
		glm::vec2(0, 1),
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		// bottom
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		glm::vec2(0, 1)
	};

	data.indices = {
		// front
		0, 1, 2,
		2, 3, 0,
		// back
		4, 5, 6,
		6, 7, 4,
		// right
		8, 9, 10,
		10, 11, 8,
		// left
		12, 13, 14,
		14, 15, 12,
		// top
		16, 17, 18,
		18, 19, 16,
		// bottom
		20, 21, 22, 
		22, 23, 20
};
    // clang-format on
    return data;
}

GeometryData Geometry::createCornellBoxGeometry(float width, float height, float depth) {
    GeometryData data;

    data.positions = {// back
                      glm::vec3(width / 2.0f, -height / 2.0f, -depth / 2.0f),
                      glm::vec3(-width / 2.0f, -height / 2.0f, -depth / 2.0f),
                      glm::vec3(-width / 2.0f, height / 2.0f, -depth / 2.0f),
                      glm::vec3(width / 2.0f, height / 2.0f, -depth / 2.0f),
                      // right
                      glm::vec3(width / 2.0f, -height / 2.0f, depth / 2.0f),
                      glm::vec3(width / 2.0f, -height / 2.0f, -depth / 2.0f),
                      glm::vec3(width / 2.0f, height / 2.0f, -depth / 2.0f),
                      glm::vec3(width / 2.0f, height / 2.0f, depth / 2.0f),
                      // left
                      glm::vec3(-width / 2.0f, -height / 2.0f, -depth / 2.0f),
                      glm::vec3(-width / 2.0f, -height / 2.0f, depth / 2.0f),
                      glm::vec3(-width / 2.0f, height / 2.0f, depth / 2.0f),
                      glm::vec3(-width / 2.0f, height / 2.0f, -depth / 2.0f),
                      // top
                      glm::vec3(-width / 2.0f, height / 2.0f, -depth / 2.0f),
                      glm::vec3(-width / 2.0f, height / 2.0f, depth / 2.0f),
                      glm::vec3(width / 2.0f, height / 2.0f, depth / 2.0f),
                      glm::vec3(width / 2.0f, height / 2.0f, -depth / 2.0f),
                      // bottom
                      glm::vec3(-width / 2.0f, -height / 2.0f, -depth / 2.0f),
                      glm::vec3(width / 2.0f, -height / 2.0f, -depth / 2.0f),
                      glm::vec3(width / 2.0f, -height / 2.0f, depth / 2.0f),
                      glm::vec3(-width / 2.0f, -height / 2.0f, depth / 2.0f)};

// clang-format off
	data.normals = {
		// back
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		// right
		glm::vec3(-1, 0, 0),
		glm::vec3(-1, 0, 0),
		glm::vec3(-1, 0, 0),
		glm::vec3(-1, 0, 0),
		// left
		glm::vec3(1, 0, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(1, 0, 0),
		// top
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0),
		// bottom
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 0)
	};

	glm::vec3 colors[5] = {
		glm::vec3(1.0, 0.0, 0.0),    // left
		glm::vec3(0.0, 1.0, 0.0),    // right
		glm::vec3(0.96, 0.93, 0.85), // top
		glm::vec3(0.64, 0.64, 0.64), // bottom
		glm::vec3(0.76, 0.74, 0.68)  // back
	};

    data.colors = {colors[4], colors[4], colors[4], colors[4],
                   colors[1], colors[1], colors[1], colors[1],
                   colors[0], colors[0], colors[0], colors[0],
                   colors[2], colors[2], colors[2], colors[2],
                   colors[3], colors[3], colors[3], colors[3]};
    // clang-format on

    // clang-format off
	data.uvs = {
		// back
		glm::vec2(1, 1),
		glm::vec2(0, 1),
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		// right
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		glm::vec2(0, 1),
		// left
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		glm::vec2(0, 1),
		// top
		glm::vec2(0, 1),
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		// bottom
		glm::vec2(0, 0),
		glm::vec2(1, 0),
		glm::vec2(1, 1),
		glm::vec2(0, 1)
	};

	data.indices = {
		// back
		2, 1, 0,
		0, 3, 2,
		// right
		6, 5, 4,
		4, 7, 6,
		// left
		10, 9, 8,
		8, 11, 10,
		// top
		14, 13, 12,
		12, 15, 14,
		// bottom
		18, 17, 16,
		16, 19, 18
	};
    // clang-format on

    return data;
}

GeometryData Geometry::createCylinderGeometry(unsigned int segments, float height, float radius) {
    GeometryData data;


    // center vertices
    data.positions.push_back(glm::vec3(0, -height / 2.0f, 0));
    data.normals.push_back(glm::vec3(0, -1, 0));
    data.uvs.push_back(glm::vec2(0.5f, 0.5f));
    data.positions.push_back(glm::vec3(0, height / 2.0f, 0));
    data.normals.push_back(glm::vec3(0, 1, 0));
    data.uvs.push_back(glm::vec2(0.5f, 0.5f));

    // circle segments
    float angle_step = 2.0f * glm::pi<float>() / float(segments);
    for (unsigned int i = 0; i < segments; i++) {
        glm::vec3 circlePos = glm::vec3(glm::cos(i * angle_step) * radius, -height / 2.0f, glm::sin(i * angle_step) * radius);

        glm::vec2 squareToCirlceUV = glm::vec2((circlePos.x / radius) * 0.5f + 0.5f, (circlePos.z / radius) * 0.5f + 0.5f);

        // bottom ring vertex
        data.positions.push_back(circlePos);
        data.positions.push_back(circlePos);
        data.normals.push_back(glm::vec3(0, -1, 0));
        data.normals.push_back(glm::normalize(circlePos - glm::vec3(0, -height / 2.0f, 0)));
        data.uvs.push_back(squareToCirlceUV);
        data.uvs.push_back(glm::vec2(i * angle_step / (2.0f * glm::pi<float>()), 0));

        // top ring vertex
        circlePos.y = height / 2.0f;
        data.positions.push_back(circlePos);
        data.positions.push_back(circlePos);
        data.normals.push_back(glm::vec3(0, 1, 0));
        data.normals.push_back(glm::normalize(circlePos - glm::vec3(0, height / 2.0f, 0)));
        data.uvs.push_back(squareToCirlceUV);
        data.uvs.push_back(glm::vec2(i * angle_step / (2.0f * glm::pi<float>()), 1));

        // bottom face
        data.indices.push_back(0);
        data.indices.push_back(2 + i * 4);
        data.indices.push_back(i == segments - 1 ? 2 : 2 + (i + 1) * 4);

        // top face
        data.indices.push_back(1);
        data.indices.push_back(i == segments - 1 ? 4 : (i + 2) * 4);
        data.indices.push_back((i + 1) * 4);

        // side faces
        data.indices.push_back(3 + i * 4);
        data.indices.push_back(i == segments - 1 ? 5 : 5 + (i + 1) * 4);
        data.indices.push_back(i == segments - 1 ? 3 : 3 + (i + 1) * 4);

        data.indices.push_back(i == segments - 1 ? 5 : 5 + (i + 1) * 4);
        data.indices.push_back(3 + i * 4);
        data.indices.push_back(5 + i * 4);
    }


    return data;
}

// Function to calculate binomial coefficient (n choose k)
int binomialCoefficient(int n, int k) {
    int result = 1;
    for (int i = 1; i <= k; ++i) {
        result *= (n - i + 1);
        result /= i;
    }
    return result;
}

// Function to calculate a point on the Bezier curve
glm::vec3 calculateBezierPoint(const std::vector<glm::vec3>& controlPoints, float t) {
    int n = controlPoints.size() - 1;
    glm::vec3 point(0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= n; ++i) {
        float blend = binomialCoefficient(n, i) * pow(t, i) * pow(1 - t, n - i);
        point += controlPoints[i] * blend;
    }
    return point;
}

// Function to generate a Bezier curve and subdivide it into N segments
std::vector<glm::vec3> generateBezierCurve(const std::vector<glm::vec3>& controlPoints, int numSegments) {
    std::vector<glm::vec3> curvePoints;
    float deltaT = 1.0f / (numSegments);
    for (int i = 0; i <= numSegments; ++i) {
        float t = i * deltaT;
        glm::vec3 point = calculateBezierPoint(controlPoints, t);
        curvePoints.push_back(point);
    }

    return curvePoints;
}

GeometryData Geometry::createBezierCylinderGeometry(unsigned int segments, std::vector<glm::vec3> controlPoints, unsigned int bezierSegments, float radius) {
    GeometryData data;
    std::vector<glm::vec3> bezierPoints = generateBezierCurve(controlPoints, bezierSegments);

    float v = 0;
    float angleStep = 2.0f * glm::pi<float>() / float(segments);
    for (int point = 0; point < bezierPoints.size(); point++) {
        glm::vec3 forwardAxis;
        if (point >= bezierPoints.size() - 1) {
            forwardAxis = -glm::normalize(bezierPoints[point - 1] - bezierPoints[point]);
        } else {
            forwardAxis = glm::normalize(bezierPoints[point + 1] - bezierPoints[point]);
        }
        glm::vec3 rightAxis = glm::normalize(glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), forwardAxis));
        glm::vec3 upAxis = glm::normalize(glm::cross(forwardAxis, rightAxis));

        // Circle segments

        unsigned int startIndex = data.positions.size();
        if (point >= bezierPoints.size() - 1) {
            v += glm::min(glm::length(bezierPoints[point - 1] - bezierPoints[point]), 1.0f);
        } else {
            v += glm::min(glm::length(bezierPoints[point + 1] - bezierPoints[point]), 1.0f);
        }
        for (unsigned int i = 0; i < segments; i++) {
            float cosTheta = glm::cos(i * angleStep);
            float sinTheta = glm::sin(i * angleStep);
            glm::vec3 circlePos = bezierPoints[point] + cosTheta * radius * rightAxis + sinTheta * radius * upAxis;
            data.positions.push_back(circlePos);
            data.normals.push_back(circlePos - bezierPoints[point]);
            float u = static_cast<float>(i) / static_cast<float>(segments);
            data.uvs.push_back(glm::vec2(u, v));
            // Side faces
            if (point < bezierPoints.size() - 1) {
                data.indices.push_back(startIndex + i);
                data.indices.push_back(startIndex + (i + 1) % segments);
                data.indices.push_back(startIndex + segments + (i + 1) % segments);

                data.indices.push_back(startIndex + segments + (i + 1) % segments);
                data.indices.push_back(startIndex + segments + i);
                data.indices.push_back(startIndex + i % segments);
            }
        }
    }
    // top face
    data.positions.push_back(bezierPoints[bezierPoints.size() - 1]);
    data.normals.push_back(bezierPoints[bezierPoints.size() - 1] - bezierPoints[bezierPoints.size() - 2]);
    data.uvs.push_back(glm::vec2(0.5f, 0.5f));
    glm::vec3 forwardAxis = glm::normalize(bezierPoints[bezierPoints.size() - 1] - bezierPoints[bezierPoints.size() - 2]);
    glm::vec3 rightAxis = glm::normalize(glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), forwardAxis));
    glm::vec3 upAxis = glm::normalize(glm::cross(forwardAxis, rightAxis));
    int numberpositions = data.positions.size() - 1;
    for (unsigned int i = 0; i <= segments; i++) {
        data.normals.push_back(bezierPoints[bezierPoints.size() - 1] - bezierPoints[bezierPoints.size() - 2]);
        glm::vec3 circlePosFlat = glm::vec3(glm::cos(i * angleStep) * radius, 0, glm::sin(i * angleStep) * radius);
        glm::vec2 squareToCirlceUV = glm::vec2((circlePosFlat.x / radius) * 0.5f + 0.5f, (circlePosFlat.z / radius) * 0.5f + 0.5f);
        data.uvs.push_back(squareToCirlceUV);
        float cosTheta = glm::cos(i * angleStep);
        float sinTheta = glm::sin(i * angleStep);

        glm::vec3 circlePos = bezierPoints[bezierPoints.size() - 1] + cosTheta * radius * rightAxis + sinTheta * radius * upAxis;
        data.positions.push_back(circlePos);
        data.indices.push_back(numberpositions + (i + 1));
        data.indices.push_back(numberpositions);
        data.indices.push_back(numberpositions + i);
    }

    // Bottom face
    data.positions.push_back(bezierPoints[0]);
    numberpositions = data.positions.size() - 1;
    forwardAxis = glm::normalize(bezierPoints[1] - bezierPoints[0]);
    rightAxis = glm::normalize(glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), forwardAxis));
    upAxis = glm::normalize(glm::cross(forwardAxis, rightAxis));
    data.normals.push_back(bezierPoints[bezierPoints.size() - 1] - bezierPoints[bezierPoints.size() - 2]);
    data.uvs.push_back(glm::vec2(0.5f, 0.5f));
    for (unsigned int i = 0; i <= segments; i++) {
        data.normals.push_back(bezierPoints[bezierPoints.size() - 1] - bezierPoints[bezierPoints.size() - 2]);
        glm::vec3 circlePosFlat = glm::vec3(glm::cos(i * angleStep) * radius, 0, glm::sin(i * angleStep) * radius);
        glm::vec2 squareToCirlceUV = glm::vec2((circlePosFlat.x / radius) * 0.5f + 0.5f, (circlePosFlat.z / radius) * 0.5f + 0.5f);
        data.uvs.push_back(squareToCirlceUV);
        float cosTheta = glm::cos(i * angleStep);
        float sinTheta = glm::sin(i * angleStep);

        glm::vec3 circlePos = bezierPoints[0] + cosTheta * radius * rightAxis + sinTheta * radius * upAxis;
        data.positions.push_back(circlePos);
        data.indices.push_back(numberpositions);
        data.indices.push_back(numberpositions + (i + 1));
        data.indices.push_back(numberpositions + i);
    }
    return data;
}

GeometryData Geometry::createSphereGeometry(unsigned int longitudeSegments, unsigned int latitudeSegments, float radius) {
    GeometryData data;


    data.positions.push_back(glm::vec3(0.0f, radius, 0.0f));
    data.positions.push_back(glm::vec3(0.0f, -radius, 0.0f));
    data.normals.push_back(glm::vec3(0.0f, radius, 0.0f));
    data.normals.push_back(glm::vec3(0.0f, -radius, 0.0f));
    data.uvs.push_back(glm::vec2(0.0f, 0.0f));
    data.uvs.push_back(glm::vec2(0.0f, 1.0f));

    // first and last ring
    for (unsigned int j = 0; j < longitudeSegments; j++) {
        data.indices.push_back(0);
        data.indices.push_back(j == longitudeSegments - 1 ? 2 : (j + 3));
        data.indices.push_back(2 + j);

        data.indices.push_back(2 + (latitudeSegments - 2) * longitudeSegments + j);
        data.indices.push_back(
            j == longitudeSegments - 1 ? 2 + (latitudeSegments - 2) * longitudeSegments
                                       : 2 + (latitudeSegments - 2) * longitudeSegments + j + 1
        );
        data.indices.push_back(1);
    }

    // vertices and rings
    for (unsigned int i = 1; i < latitudeSegments; i++) {
        float verticalAngle = float(i) * glm::pi<float>() / float(latitudeSegments);
        for (unsigned int j = 0; j < longitudeSegments; j++) {
            float horizontalAngle = float(j) * 2.0f * glm::pi<float>() / float(longitudeSegments);
            glm::vec3 position = glm::vec3(
                radius * glm::sin(verticalAngle) * glm::cos(horizontalAngle),
                radius * glm::cos(verticalAngle),
                radius * glm::sin(verticalAngle) * glm::sin(horizontalAngle)
            );
            data.positions.push_back(position);
            data.normals.push_back(glm::normalize(position));
            data.uvs.push_back(glm::vec2(horizontalAngle / (2.0f * glm::pi<float>()), verticalAngle / glm::pi<float>()));

            if (i == 1)
                continue;

            data.indices.push_back(2 + (i - 1) * longitudeSegments + j);
            data.indices.push_back(j == longitudeSegments - 1 ? 2 + (i - 2) * longitudeSegments : 2 + (i - 2) * longitudeSegments + j + 1);
            data.indices.push_back(j == longitudeSegments - 1 ? 2 + (i - 1) * longitudeSegments : 2 + (i - 1) * longitudeSegments + j + 1);

            data.indices.push_back(j == longitudeSegments - 1 ? 2 + (i - 2) * longitudeSegments : 2 + (i - 2) * longitudeSegments + j + 1);
            data.indices.push_back(2 + (i - 1) * longitudeSegments + j);
            data.indices.push_back(2 + (i - 2) * longitudeSegments + j);
        }
    }

    return data;
}
*/


