#pragma once

#include <string>
#include <vector>
#include <functional>

namespace cosmic {
namespace core {

/**
 * @brief Single tutorial step
 */
struct TutorialStep {
    std::string title;
    std::string description;
    std::string action;           // What the user should do
    std::string highlightUI;      // UI element to highlight (optional)
    float duration;               // Minimum time on this step (seconds), 0 = manual
    bool completed;

    // Optional callback when step is shown
    std::function<void()> onShow;
    // Optional callback to check if step is complete
    std::function<bool()> isComplete;
};

/**
 * @brief Tutorial lesson (collection of steps)
 */
struct TutorialLesson {
    std::string name;
    std::string description;
    std::vector<TutorialStep> steps;
    bool completed;
};

/**
 * @brief Interactive tutorial system
 *
 * Guides users through the simulator with step-by-step instructions.
 * Topics covered:
 * - Basic navigation and controls
 * - Understanding black holes (Schwarzschild vs Kerr)
 * - Accretion disks and relativistic jets
 * - Gravitational lensing and photon spheres
 * - Measurement tools
 * - Particle trajectories
 * - Neutron stars and pulsars
 * - Gravitational waves
 */
class TutorialSystem {
public:
    TutorialSystem();

    /**
     * @brief Initialize tutorials
     */
    void initialize();

    /**
     * @brief Update tutorial state
     */
    void update(float deltaTime);

    /**
     * @brief Check if tutorial is active
     */
    bool isActive() const { return active_; }

    /**
     * @brief Start tutorial
     */
    void start();

    /**
     * @brief Stop tutorial
     */
    void stop();

    /**
     * @brief Select a specific lesson
     */
    void selectLesson(int lessonIndex);

    /**
     * @brief Go to next step
     */
    void nextStep();

    /**
     * @brief Go to previous step
     */
    void previousStep();

    /**
     * @brief Skip current lesson
     */
    void skipLesson();

    /**
     * @brief Get current lesson
     */
    const TutorialLesson* getCurrentLesson() const;

    /**
     * @brief Get current step
     */
    const TutorialStep* getCurrentStep() const;

    /**
     * @brief Get all lessons
     */
    const std::vector<TutorialLesson>& getLessons() const { return lessons_; }

    /**
     * @brief Get current lesson index
     */
    int getCurrentLessonIndex() const { return currentLessonIndex_; }

    /**
     * @brief Get current step index
     */
    int getCurrentStepIndex() const { return currentStepIndex_; }

    /**
     * @brief Get progress (0.0 to 1.0)
     */
    float getProgress() const;

    /**
     * @brief Mark current step as completed
     */
    void markStepComplete();

    /**
     * @brief Check if all tutorials completed
     */
    bool allCompleted() const;

private:
    bool active_;
    std::vector<TutorialLesson> lessons_;
    int currentLessonIndex_;
    int currentStepIndex_;
    float timeOnCurrentStep_;

    /**
     * @brief Create individual tutorial lessons
     */
    void createBasicsLesson();
    void createBlackHoleLesson();
    void createVisualizationsLesson();
    void createMeasurementsLesson();
    void createParticlesLesson();
    void createAdvancedLesson();
};

} // namespace core
} // namespace cosmic
