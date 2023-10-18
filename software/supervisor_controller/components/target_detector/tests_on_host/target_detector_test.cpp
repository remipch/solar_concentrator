#include "mini_mock.hpp"

#undef NDEBUG

#include "target_detector.hpp"

CImg<unsigned char> load_image_as_grayscale(const char* image_path) {
    CImg<unsigned char> image(image_path);
    if(image.spectrum()>=3) {
        return image.get_RGBtoYCbCr().get_channel(0);
    }
    return image;
}

bool detect_from_file(const char* image_path, rectangle_t& target) {
    target_detector_init();

    CImg<unsigned char> image = load_image_as_grayscale(image_path);

    bool result = target_detector_detect(image, target);

    // Save output for manual debug only
    // (in final application, the output image is used for display purpose only)
    image.save("output.jpg");

    return result;
}

TEST(when_capstones_are_missing_then_detection_fails, []()
{
    rectangle_t target_area;
    bool target_detected = detect_from_file("missing_capstones.jpg", target_area);
    EXPECT(!target_detected);
});

TEST(when_capstones_are_misaligned_then_detection_fails, []()
{
    rectangle_t target_area;
    bool target_detected = detect_from_file("misaligned_capstones.jpg", target_area);
    EXPECT(!target_detected);
});

TEST(when_capstones_are_correct_then_area_is_detected, []()
{
    rectangle_t target_area;
    bool target_detected = detect_from_file("correct_capstones.jpg", target_area);
    EXPECT(target_detected);
});

CREATE_MAIN_ENTRY_POINT();

