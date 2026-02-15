#ifndef RENDEREABLE_H
#define RENDEREABLE_H

class IPage : public std::enable_shared_from_this<IPage>
{
    friend class MediaFramework;

public:
    IPage() = default;
    virtual ~IPage() = default;
    virtual void Initialize() { bIsInitialized = true; }
    virtual void Start() = 0;

protected:
    bool bIsInitialized = false;

    virtual void RenderStage() = 0;
};

#endif