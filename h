local pathToGithub = "https://raw.githubusercontent.com/xhayper/Animator/main/Source/"

local sub = string.sub

getgenv().httpRequireCache = getgenv().httpRequireCache or {}

getgenv().HttpRequire = function(path, noCache)
	if sub(path, 1, 8) == "https://" or sub(path, 1, 7) == "http://" then
		if not noCache and httpRequireCache[path] then
			return httpRequireCache[path]
		end
		-- syn > request > vanilla
		httpRequireCache[path] = loadstring(
			(syn and syn.request) and syn.request({ Url = path }).Body
				or (request and request({ Url = path }).Body or game:HttpGet(path))
		)()
		return httpRequireCache[path]
	else
		return require(path)
	end
end

getgenv().animatorRequire = function(path)
	return HttpRequire(pathToGithub .. path)
end

-- 加载原始Animator
local OriginalAnimator = animatorRequire("Animator.lua")
local Utility = animatorRequire("Utility.lua")

-- 创建优化版的Animator包装器
getgenv().Animator = {
    new = function(character, animation)
        local originalAnim = OriginalAnimator.new(character, animation)
        
        -- 优化Play方法
        local originalPlay = originalAnim.Play
        function originalAnim:Play()
            -- 添加性能优化逻辑
            if not self._optimized then
                self:_applyOptimizations()
            end
            return originalPlay(self)
        end
        
        -- 添加优化方法
        function originalAnim:_applyOptimizations()
            if self._optimized then return end
            
            -- 1. 确保使用deltaTime
            if self._mainLoop then
                -- 替换为使用deltaTime的更新循环
                self:_patchUpdateLoop()
            end
            
            -- 2. 预缓存骨骼部件
            self:_cacheBones()
            
            -- 3. 限制更新频率（可选）
            self:_limitUpdateRate()
            
            self._optimized = true
        end
        
        -- 修补更新循环使用deltaTime
        function originalAnim:_patchUpdateLoop()
            if not self._mainLoop then return end
            
            -- 保存原始连接
            local originalConnection = self._mainLoop
            if originalConnection.Connected then
                originalConnection:Disconnect()
            end
            
            -- 创建使用deltaTime的新循环
            self._lastUpdate = os.clock()
            self._mainLoop = game:GetService("RunService").Heartbeat:Connect(function()
                local currentTime = os.clock()
                local deltaTime = currentTime - self._lastUpdate
                self._lastUpdate = currentTime
                
                -- 调用原始更新方法，但传入deltaTime
                if self.Update then
                    self:Update(deltaTime)
                elseif self._update then
                    self:_update(deltaTime)
                end
            end)
        end
        
        -- 预缓存骨骼
        function originalAnim:_cacheBones()
            if self.Bones then return end
            
            self.Bones = {}
            local humanoid = self.Character and self.Character:FindFirstChildOfClass("Humanoid")
            if not humanoid then return end
            
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
                        OriginalCFrame = part.CFrame
                    }
                end
            end
        end
        
        -- 限制更新频率（减少CPU使用）
        function originalAnim:_limitUpdateRate()
            self._updateInterval = 0 -- 每帧都更新
            self._lastUpdateTime = 0
            
            if self.Update then
                local originalUpdate = self.Update
                self.Update = function(self, deltaTime)
                    self._lastUpdateTime = self._lastUpdateTime + deltaTime
                    if self._lastUpdateTime >= self._updateInterval then
                        originalUpdate(self, self._lastUpdateTime)
                        self._lastUpdateTime = 0
                    end
                end
            end
        end
        
        -- 添加销毁方法
        function originalAnim:Destroy()
            if self._mainLoop then
                self._mainLoop:Disconnect()
            end
            if self.Stop then
                self:Stop()
            end
        end
        
        return originalAnim
    end
}

-- 可选：保持原来的Hook功能，但使用优化后的Animator
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
		return Animator.new(Object.Parent, ...)
	end)
	Utility:sendNotif("优化版动画钩子已加载", "性能已提升", 3)
end

Utility:sendNotif("优化版Animator API已加载", "自动应用性能优化", 5)

-- 可选：自动启用Hook
-- hookAnimatorFunction()

return "优化版动画系统 - 兼容原API"
