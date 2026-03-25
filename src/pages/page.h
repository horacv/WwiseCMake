#ifndef RENDEREABLE_H
#define RENDEREABLE_H

class IPage : public std::enable_shared_from_this<IPage>
{
    friend class MediaFramework;

public:
    IPage() = default;
    virtual ~IPage() = default;
    virtual void Initialize()
    {
        bIsInitialized.store(true, std::memory_order_release);
    }
    virtual void Deinitialize()
    {
        bIsInitialized.store(false, std::memory_order_release);
    }

    virtual bool IsInitialized() const { return bIsInitialized; }

    virtual bool CanClose() { return true; }

protected:
    std::atomic<bool> bIsInitialized = false;

    virtual void Start() = 0;
    virtual void RenderStage() = 0;
};

#endif