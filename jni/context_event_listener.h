#include <core/environment.h>
#include <jni.h>

namespace Beatmup {

    /**
     * Needed for correct functioning of JNI
     */
    class ContextEventListener : public Beatmup::Environment::EventListener {
    private:
        JavaVM *jvm;

    public:
        ContextEventListener(JNIEnv *jenv);

        void threadCreated(PoolIndex pool);

        void threadTerminating(PoolIndex pool);

        void taskFail(PoolIndex pool, Beatmup::AbstractTask &task, const std::exception &ex);

        void gpuInitFail(PoolIndex pool, const std::exception &ex);

        bool taskDone(PoolIndex pool, Beatmup::AbstractTask &task, bool aborted);
    };

}