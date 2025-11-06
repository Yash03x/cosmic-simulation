#include "core/Tutorial.hpp"

namespace cosmic {
namespace core {

TutorialSystem::TutorialSystem()
    : active_(false),
      currentLessonIndex_(0),
      currentStepIndex_(0),
      timeOnCurrentStep_(0.0f) {
}

void TutorialSystem::initialize() {
    lessons_.clear();

    // Create all tutorial lessons
    createBasicsLesson();
    createBlackHoleLesson();
    createVisualizationsLesson();
    createMeasurementsLesson();
    createParticlesLesson();
    createAdvancedLesson();
}

void TutorialSystem::update(float deltaTime) {
    if (!active_) return;

    timeOnCurrentStep_ += deltaTime;

    // Auto-advance if step has duration and is complete
    auto step = getCurrentStep();
    if (step && step->duration > 0 && timeOnCurrentStep_ >= step->duration) {
        if (!step->isComplete || step->isComplete()) {
            markStepComplete();
            nextStep();
        }
    }
}

void TutorialSystem::start() {
    active_ = true;
    currentLessonIndex_ = 0;
    currentStepIndex_ = 0;
    timeOnCurrentStep_ = 0.0f;

    // Trigger onShow callback for first step
    auto step = getCurrentStep();
    if (step && step->onShow) {
        step->onShow();
    }
}

void TutorialSystem::stop() {
    active_ = false;
}

void TutorialSystem::selectLesson(int lessonIndex) {
    if (lessonIndex >= 0 && lessonIndex < static_cast<int>(lessons_.size())) {
        currentLessonIndex_ = lessonIndex;
        currentStepIndex_ = 0;
        timeOnCurrentStep_ = 0.0f;

        auto step = getCurrentStep();
        if (step && step->onShow) {
            step->onShow();
        }
    }
}

void TutorialSystem::nextStep() {
    if (currentLessonIndex_ >= static_cast<int>(lessons_.size())) return;

    auto& lesson = lessons_[currentLessonIndex_];

    if (currentStepIndex_ < static_cast<int>(lesson.steps.size()) - 1) {
        currentStepIndex_++;
        timeOnCurrentStep_ = 0.0f;

        auto step = getCurrentStep();
        if (step && step->onShow) {
            step->onShow();
        }
    } else {
        // End of lesson, mark as complete and move to next
        lesson.completed = true;

        if (currentLessonIndex_ < static_cast<int>(lessons_.size()) - 1) {
            currentLessonIndex_++;
            currentStepIndex_ = 0;
            timeOnCurrentStep_ = 0.0f;

            auto step = getCurrentStep();
            if (step && step->onShow) {
                step->onShow();
            }
        } else {
            // All lessons complete
            stop();
        }
    }
}

void TutorialSystem::previousStep() {
    if (currentStepIndex_ > 0) {
        currentStepIndex_--;
        timeOnCurrentStep_ = 0.0f;

        auto step = getCurrentStep();
        if (step && step->onShow) {
            step->onShow();
        }
    } else if (currentLessonIndex_ > 0) {
        // Go to previous lesson
        currentLessonIndex_--;
        currentStepIndex_ = static_cast<int>(lessons_[currentLessonIndex_].steps.size()) - 1;
        timeOnCurrentStep_ = 0.0f;

        auto step = getCurrentStep();
        if (step && step->onShow) {
            step->onShow();
        }
    }
}

void TutorialSystem::skipLesson() {
    if (currentLessonIndex_ < static_cast<int>(lessons_.size()) - 1) {
        lessons_[currentLessonIndex_].completed = true;
        currentLessonIndex_++;
        currentStepIndex_ = 0;
        timeOnCurrentStep_ = 0.0f;

        auto step = getCurrentStep();
        if (step && step->onShow) {
            step->onShow();
        }
    } else {
        stop();
    }
}

const TutorialLesson* TutorialSystem::getCurrentLesson() const {
    if (currentLessonIndex_ >= 0 && currentLessonIndex_ < static_cast<int>(lessons_.size())) {
        return &lessons_[currentLessonIndex_];
    }
    return nullptr;
}

const TutorialStep* TutorialSystem::getCurrentStep() const {
    auto lesson = getCurrentLesson();
    if (lesson && currentStepIndex_ >= 0 && currentStepIndex_ < static_cast<int>(lesson->steps.size())) {
        return &lesson->steps[currentStepIndex_];
    }
    return nullptr;
}

float TutorialSystem::getProgress() const {
    if (lessons_.empty()) return 1.0f;

    int totalSteps = 0;
    int completedSteps = 0;

    for (const auto& lesson : lessons_) {
        totalSteps += static_cast<int>(lesson.steps.size());
        if (lesson.completed) {
            completedSteps += static_cast<int>(lesson.steps.size());
        }
    }

    // Add current progress
    completedSteps += currentStepIndex_;

    return static_cast<float>(completedSteps) / static_cast<float>(totalSteps);
}

void TutorialSystem::markStepComplete() {
    if (currentLessonIndex_ >= 0 && currentLessonIndex_ < static_cast<int>(lessons_.size())) {
        auto& lesson = lessons_[currentLessonIndex_];
        if (currentStepIndex_ >= 0 && currentStepIndex_ < static_cast<int>(lesson.steps.size())) {
            lesson.steps[currentStepIndex_].completed = true;
        }
    }
}

bool TutorialSystem::allCompleted() const {
    for (const auto& lesson : lessons_) {
        if (!lesson.completed) return false;
    }
    return true;
}

// ==================== Lesson Creators ====================

void TutorialSystem::createBasicsLesson() {
    TutorialLesson lesson;
    lesson.name = "Getting Started";
    lesson.description = "Learn basic controls and navigation";
    lesson.completed = false;

    TutorialStep step1;
    step1.title = "Welcome!";
    step1.description = "Welcome to the Cosmic Black Hole Simulator! This tutorial will guide you through the basics.";
    step1.action = "Click 'Next' to continue";
    step1.duration = 0.0f;
    step1.completed = false;
    lesson.steps.push_back(step1);

    TutorialStep step2;
    step2.title = "Camera Movement";
    step2.description = "Use W/A/S/D to fly around. Q/E to move up/down. Hold SHIFT for speed boost.";
    step2.action = "Try moving the camera around";
    step2.duration = 0.0f;
    step2.completed = false;
    lesson.steps.push_back(step2);

    TutorialStep step3;
    step3.title = "Mouse Look";
    step3.description = "Press TAB to capture the mouse, then move it to look around.";
    step3.action = "Press TAB and look around";
    step3.duration = 0.0f;
    step3.completed = false;
    lesson.steps.push_back(step3);

    TutorialStep step4;
    step4.title = "Camera Presets";
    step4.description = "Press 1-9 to instantly jump to preset camera positions. Try pressing 2 for a close-up view!";
    step4.action = "Press 2 to get close to the black hole";
    step4.duration = 0.0f;
    step4.completed = false;
    lesson.steps.push_back(step4);

    TutorialStep step5;
    step5.title = "UI Toggle";
    step5.description = "Press H to hide/show the user interface. Press P to print statistics to console.";
    step5.action = "Try pressing H to toggle the UI";
    step4.duration = 0.0f;
    step5.completed = false;
    lesson.steps.push_back(step5);

    lessons_.push_back(lesson);
}

void TutorialSystem::createBlackHoleLesson() {
    TutorialLesson lesson;
    lesson.name = "Black Hole Physics";
    lesson.description = "Understanding black holes and spacetime";
    lesson.completed = false;

    TutorialStep step1;
    step1.title = "Black Hole Basics";
    step1.description = "A black hole is a region where gravity is so strong that nothing can escape, not even light!";
    step1.action = "Continue";
    step1.duration = 0.0f;
    step1.completed = false;
    lesson.steps.push_back(step1);

    TutorialStep step2;
    step2.title = "Event Horizon";
    step2.description = "The event horizon is the boundary of no return. Once you cross it, you can never escape. For a non-rotating black hole, r = 2GM/c².";
    step2.action = "Observe the dark sphere at the center";
    step2.duration = 0.0f;
    step2.completed = false;
    lesson.steps.push_back(step2);

    TutorialStep step3;
    step3.title = "Schwarzschild vs Kerr";
    step3.description = "Schwarzschild black holes don't rotate. Kerr black holes do! Rotation affects the event horizon shape and creates an ergosphere.";
    step3.action = "Open 'Black Hole Settings' and try changing the Spin parameter";
    step3.duration = 0.0f;
    step3.completed = false;
    lesson.steps.push_back(step3);

    TutorialStep step4;
    step4.title = "Famous Black Holes";
    step4.description = "Use the 'Famous Black Holes' dropdown to explore real black holes like M87* (first ever photographed) and Sgr A* (our galaxy's center).";
    step4.action = "Select a black hole preset from the dropdown";
    step4.duration = 0.0f;
    step4.completed = false;
    lesson.steps.push_back(step4);

    lessons_.push_back(lesson);
}

void TutorialSystem::createVisualizationsLesson() {
    TutorialLesson lesson;
    lesson.name = "Visual Features";
    lesson.description = "Explore different visualization options";
    lesson.completed = false;

    TutorialStep step1;
    step1.title = "Accretion Disk";
    step1.description = "The glowing disk around the black hole is matter spiraling inward, heating up to millions of degrees!";
    step1.action = "Toggle 'Accretion Disk' in the UI";
    step1.duration = 0.0f;
    step1.completed = false;
    lesson.steps.push_back(step1);

    TutorialStep step2;
    step2.title = "Relativistic Jets";
    step2.description = "Spinning black holes can launch jets of plasma at near light speed! These are powered by magnetic fields.";
    step2.action = "Enable 'Relativistic Jets' checkbox";
    step2.duration = 0.0f;
    step2.completed = false;
    lesson.steps.push_back(step2);

    TutorialStep step3;
    step3.title = "Ergosphere";
    step3.description = "The ergosphere is unique to rotating black holes. Inside it, spacetime itself is dragged around!";
    step3.action = "Enable 'Ergosphere' visualization";
    step3.duration = 0.0f;
    step3.completed = false;
    lesson.steps.push_back(step3);

    TutorialStep step4;
    step4.title = "Tidal Forces";
    step4.description = "Get too close and tidal forces will tear you apart! This is called spaghettification.";
    step4.action = "Enable 'Tidal Forces' to see danger zones";
    step4.duration = 0.0f;
    step4.completed = false;
    lesson.steps.push_back(step4);

    lessons_.push_back(lesson);
}

void TutorialSystem::createMeasurementsLesson() {
    TutorialLesson lesson;
    lesson.name = "Measurement Tools";
    lesson.description = "Measure distances and physical quantities";
    lesson.completed = false;

    TutorialStep step1;
    step1.title = "Measurement Tools";
    step1.description = "You can measure distances, angles, and gravitational redshift in curved spacetime!";
    step1.action = "Open the 'Measurement Tools' panel";
    step1.duration = 0.0f;
    step1.completed = false;
    lesson.steps.push_back(step1);

    TutorialStep step2;
    step2.title = "Distance Measurements";
    step2.description = "Distances in curved spacetime differ from flat-space distances. The measurement tool shows both!";
    step2.action = "Try adding distance measurement points";
    step2.duration = 0.0f;
    step2.completed = false;
    lesson.steps.push_back(step2);

    TutorialStep step3;
    step3.title = "Gravitational Redshift";
    step3.description = "Light loses energy climbing out of a gravitational well. This shifts it to redder wavelengths.";
    step3.action = "Measure redshift between two points";
    step3.duration = 0.0f;
    step3.completed = false;
    lesson.steps.push_back(step3);

    lessons_.push_back(lesson);
}

void TutorialSystem::createParticlesLesson() {
    TutorialLesson lesson;
    lesson.name = "Particle Trajectories";
    lesson.description = "Simulate particles and light rays";
    lesson.completed = false;

    TutorialStep step1;
    step1.title = "Geodesics";
    step1.description = "Particles follow curved paths called geodesics in curved spacetime. Let's visualize them!";
    step1.action = "Open 'Particle Trajectories' panel";
    step1.duration = 0.0f;
    step1.completed = false;
    lesson.steps.push_back(step1);

    TutorialStep step2;
    step2.title = "Circular Orbits";
    step2.description = "Particles can orbit black holes! The innermost stable circular orbit (ISCO) is at 6M for non-rotating black holes.";
    step2.action = "Click 'ISCO Orbit' to add an orbit";
    step2.duration = 0.0f;
    step2.completed = false;
    lesson.steps.push_back(step2);

    TutorialStep step3;
    step3.title = "Photon Paths";
    step3.description = "Light is bent by gravity! Launch photons to see gravitational lensing in action.";
    step3.action = "Launch a photon ring to see lensing";
    step3.duration = 0.0f;
    step3.completed = false;
    lesson.steps.push_back(step3);

    lessons_.push_back(lesson);
}

void TutorialSystem::createAdvancedLesson() {
    TutorialLesson lesson;
    lesson.name = "Advanced Features";
    lesson.description = "Neutron stars, pulsars, and gravitational waves";
    lesson.completed = false;

    TutorialStep step1;
    step1.title = "Neutron Stars";
    step1.description = "Neutron stars are incredibly dense - a teaspoon weighs a billion tons! They're the remnants of massive stars.";
    step1.action = "Open 'Neutron Stars' panel";
    step1.duration = 0.0f;
    step1.completed = false;
    lesson.steps.push_back(step1);

    TutorialStep step2;
    step2.title = "Pulsars";
    step2.description = "Pulsars are rapidly rotating neutron stars with lighthouse-like beams. Some spin hundreds of times per second!";
    step2.action = "Enable 'Show Lighthouse Beams' for pulsar mode";
    step2.duration = 0.0f;
    step2.completed = false;
    lesson.steps.push_back(step2);

    TutorialStep step3;
    step3.title = "Gravitational Waves";
    step3.description = "When massive objects merge, they create ripples in spacetime itself! LIGO detected these for the first time in 2015.";
    step3.action = "Open 'Gravitational Waves' panel";
    step3.duration = 0.0f;
    step3.completed = false;
    lesson.steps.push_back(step3);

    TutorialStep step4;
    step4.title = "Time Controls";
    step4.description = "You can pause the simulation or speed it up 10x! Great for watching slow phenomena.";
    step4.action = "Try the time controls (Play/Pause, Speed)";
    step4.duration = 0.0f;
    step4.completed = false;
    lesson.steps.push_back(step4);

    TutorialStep step5;
    step5.title = "Screenshots";
    step5.description = "Press F12 to save a screenshot of your view. Perfect for sharing cool discoveries!";
    step5.action = "Press F12 to take a screenshot";
    step5.duration = 0.0f;
    step5.completed = false;
    lesson.steps.push_back(step5);

    TutorialStep step6;
    step6.title = "Tutorial Complete!";
    step6.description = "You've learned the basics! Now explore and experiment. Science awaits!";
    step6.action = "Start exploring on your own";
    step6.duration = 0.0f;
    step6.completed = false;
    lesson.steps.push_back(step6);

    lessons_.push_back(lesson);
}

} // namespace core
} // namespace cosmic
