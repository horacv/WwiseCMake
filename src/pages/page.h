#ifndef RENDEREABLE_H
#define RENDEREABLE_H

class IPage : public std::enable_shared_from_this<IPage>
{
    friend class MediaFramework;

public:
    IPage() = default;
    virtual ~IPage() = default;

    virtual void Initialize() { bIsInitialized.store(true, std::memory_order_release); }
    virtual void Deinitialize() { bIsInitialized.store(false, std::memory_order_release); }

    virtual bool IsInitialized() const { return bIsInitialized.load(std::memory_order_relaxed); }
    virtual bool CanDestroy() { return bCanDestroy.load(std::memory_order_relaxed); }

protected:
    std::atomic<bool> bIsInitialized = false;
    std::atomic<bool> bCanDestroy = true;

    virtual void Start() = 0;
    virtual void RenderStage() = 0;
};

#endif