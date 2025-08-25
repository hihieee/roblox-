local OptimizedAnimator = {}

-- 优化版的Animator类
function OptimizedAnimator.new(character, animation)
    local self = setmetatable({}, { __index = OptimizedAnimator })
    
    self.Character = character
    self.Animation = animation
    self.IsPlaying = false
    self.TimePosition = 0
    self.WeightCurrent = 0
    self.WeightTarget = 1
    self.FadeTime = 0.2
    
    -- 使用一个连接器而不是多个
    self._connection = nil
    self._lastUpdate = os.clock()
    
    -- 预缓存骨骼部件
    self:cacheBones()
    
    return self
end

-- 预缓存骨骼部件以提高性能
function OptimizedAnimator:cacheBones()
    self.Bones = {}
    local humanoid = self.Character:FindFirstChildOfClass("Humanoid")
    if not humanoid then return end
    
    -- 只缓存需要的骨骼，而不是所有部件
    local boneNames = {
        "Head", "UpperTorso", "LowerTorso", "LeftUpperArm", "LeftLowerArm", "LeftHand",
        "RightUpperArm", "RightLowerArm", "RightHand", "LeftUpperLeg", "LeftLowerLeg", "LeftFoot",
        "RightUpperLeg", "RightLowerLeg", "RightFoot"
    }
    
    for _, name in ipairs(boneNames) do
        local part = self.Character:FindFirstChild(name)
        if part then
            self.Bones[name] = {
                Part = part,
                OriginalCFrame = part.CFrame,
                OriginalSize = part.Size
            }
        end
    end
end

-- 优化的播放方法
function OptimizedAnimator:Play()
    if self.IsPlaying then return end
    
    self.IsPlaying = true
    self.TimePosition = 0
    self.WeightCurrent = 0
    
    -- 使用Heartbeat而不是RenderStepped，减少性能开销
    if self._connection then
        self._connection:Disconnect()
    end
    
    self._connection = game:GetService("RunService").Heartbeat:Connect(function()
        self:update()
    end)
end

-- 优化的停止方法
function OptimizedAnimator:Stop()
    self.IsPlaying = false
    if self._connection then
        self._connection:Disconnect()
        self._connection = nil
    end
    self:resetBones()
end

-- 核心优化：使用deltaTime确保帧率无关
function OptimizedAnimator:update()
    local currentTime = os.clock()
    local deltaTime = currentTime - self._lastUpdate
    self._lastUpdate = currentTime
    
    if not self.IsPlaying then return end
    
    -- 更新时间位置
    self.TimePosition = self.TimePosition + deltaTime
    
    -- 平滑权重过渡
    self.WeightCurrent = math.min(self.WeightCurrent + (deltaTime / self.FadeTime), self.WeightTarget)
    
    -- 应用动画姿势（这里需要根据实际动画数据来）
    self:applyPose(deltaTime)
end

-- 优化的姿势应用
function OptimizedAnimator:applyPose(deltaTime)
    -- 这里是简化的示例，实际需要根据动画数据来计算
    for boneName, boneData in pairs(self.Bones) do
        if boneData.Part and boneData.Part.Parent then
            -- 使用CFrame.lookAt等高效方法而不是复杂的数学运算
            local oscillation = math.sin(self.TimePosition * 2) * 0.1
            local offset = Vector3.new(oscillation, 0, 0)
            
            -- 使用lerp平滑过渡
            boneData.Part.CFrame = boneData.OriginalCFrame:lerp(
                CFrame.new(boneData.OriginalCFrame.Position + offset),
                self.WeightCurrent * 0.5
            )
        end
    end
end

-- 重置骨骼到原始位置
function OptimizedAnimator:resetBones()
    for boneName, boneData in pairs(self.Bones) do
        if boneData.Part and boneData.Part.Parent then
            boneData.Part.CFrame = boneData.OriginalCFrame
        end
    end
end

-- 销毁方法
function OptimizedAnimator:Destroy()
    self:Stop()
    self.Bones = nil
end

-- 替换原来的Hook系统，但使用我们优化的Animator
getgenv().hookAnimatorFunction = function()
    local OldFunc
    OldFunc = hookmetamethod(game, "__namecall", function(Object, ...)
        local NamecallMethod = getnamecallmethod()
        if not checkcaller() or Object.ClassName ~= "Humanoid" or NamecallMethod ~= "LoadAnimation" then
            return OldFunc(Object, ...)
        end
        local args = { ... }
        if args[2] then
            return OldFunc(Object, ...)
        end
        -- 使用我们优化的Animator而不是GitHub上的
        return OptimizedAnimator.new(Object.Parent, ...)
    end)
    
    game:GetService("StarterGui"):SetCore("SendNotification", {
        Title = "优化版动画钩子已加载",
        Text = "性能已优化",
        Duration = 3
    })
end

-- 提供API
getgenv().Animator = OptimizedAnimator
getgenv().OptimizedAnimator = OptimizedAnimator

game:GetService("StarterGui"):SetCore("SendNotification", {
    Title = "优化版动画系统已加载",
    Text = "使用hookAnimatorFunction()启用",
    Duration = 5
})

-- 可选：自动启用Hook
-- hookAnimatorFunction()

return "优化版动画系统 - 性能修复"
-- 添加禁用Hook的功能
getgenv().disableAnimatorHook = function()
    if not getgenv()._animatorHookEnabled then
        Utility:sendNotif("Hook not enabled", nil, 3)
        return
    end
    
    if getgenv()._originalAnimatorHook then
        hookmetamethod(game, "__namecall", getgenv()._originalAnimatorHook)
    end
    
    getgenv()._animatorHookEnabled = false
    Utility:sendNotif("Reverted to Native Animation System", "Animation performance restored", 5)
end

-- 添加选择性使用自定义动画的方法
getgenv().createCustomAnimation = function(humanoid, animationId)
    if not humanoid or not animationId then
        error("Invalid arguments: humanoid and animationId required")
    end
    
    local animation = Instance.new("Animation")
    animation.AnimationId = animationId
    
    -- 使用自定义Animator创建动画
    return Animator.new(humanoid.Parent, animation)
end

Utility:sendNotif("Animator API Loaded", "Native system active. Use hookAnimatorFunction() to enable custom animator", 5)

-- 不自动启用Hook，保持原生系统
-- hookAnimatorFunction() -- 这行被注释掉了

return {
    Animator = Animator,
    enableCustomAnimator = hookAnimatorFunction,
    disableCustomAnimator = disableAnimatorHook,
    createCustomAnimation = createCustomAnimation,
    Utility = Utility
}
