// 获取语言切换按钮
const switchToEn = document.getElementById('switch-to-en');
const switchToCn = document.getElementById('switch-to-cn');
// 获取内容区域
const content = document.getElementById('content');

// 切换到英文
switchToEn.addEventListener('click', () => {
    const enElements = content.querySelectorAll(':not(.cn)');
    const cnElements = content.querySelectorAll('.cn');
    enElements.forEach(element => element.style.display = 'block');
    cnElements.forEach(element => element.style.display = 'none');
});

// 切换到中文
switchToCn.addEventListener('click', () => {
    const enElements = content.querySelectorAll(':not(.cn)');
    const cnElements = content.querySelectorAll('.cn');
    enElements.forEach(element => element.style.display = 'none');
    cnElements.forEach(element => element.style.display = 'block');
});