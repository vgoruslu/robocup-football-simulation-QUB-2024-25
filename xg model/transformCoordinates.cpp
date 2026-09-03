#include <iostream>

struct Point {
    double x, y;
};

Point transformCoordinates(const Point& input) {
    // Source pitch dimensions
    const double sourceWidth = 120.0;
    const double sourceHeight = 80.0;

    // Target pitch dimensions
    const double targetWidth = 104.0;
    const double targetHeight = 66.0;

    Point output;

    // Scale and shift origin
    output.x = (input.x / sourceWidth) * targetWidth - (targetWidth / 2.0);
    output.y = (targetHeight / 2.0) - (input.y / sourceHeight) * targetHeight;

    return output;
}

int main() {
    // Example input points
    /*Point corners[] = {
        {0, 0},        // Bottom-left corner
        {120, 0},      // Bottom-right corner
        {0, 80},       // Top-left corner
        {120, 80}      // Top-right corner
    };

    for (const auto& corner : corners) {
        Point transformed = transformCoordinates(corner);
        std::cout << "Original: (" << corner.x << ", " << corner.y << ") -> "
            << "Transformed: (" << transformed.x << ", " << transformed.y << ")\n";
    }
    */

    Point testPoint = { 50,50 };
    Point transformedPoint = transformCoordinates(testPoint);
    std::cout << "Original: (" << testPoint.x << ", " << testPoint.y << ") -> "
        << "Transformed: (" << transformedPoint.x << ", " << transformedPoint.y << ")\n";

    return 0;
}
