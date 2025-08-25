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

-- 加载Animator但不自动Hook
getgenv().Animator = animatorRequire("Animator.lua")
local Utility = animatorRequire("Utility.lua")

-- 保留Hook功能，但不自动执行，让用户选择何时使用
getgenv().hookAnimatorFunction = function()
    if getgenv()._animatorHookEnabled then
        Utility:sendNotif("Hook already enabled", nil, 3)
        return
    end
    
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
	
	getgenv()._animatorHookEnabled = true
	getgenv()._originalAnimatorHook = OldFunc
	Utility:sendNotif("Custom Animator Hook Loaded", "Use disableAnimatorHook() to revert to native system", 5)
end

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
