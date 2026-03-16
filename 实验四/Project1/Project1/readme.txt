实验四：
一、程序功能说明
	本程序基于 OpenGL + GLSL + FreeGLUT 实现一个飞行地形场景，主要功能如下：
	地形绘制与纹理映射
   		从 obj 文件加载地形网格数据（顶点、面、纹理坐标），
   		从 ppm 文件加载纹理图像，并在 GLSL 片元着色器中完成纹理映射，
  		 地形能够正确显示起伏结构和纹理细节。
	飞行模拟
  		采用飞行员视角在地形上空漫游，支持完整的飞机姿态控制：
  		左 / 右方向键：偏航（Yaw）
  		上 / 下方向键：俯仰（Pitch）
   		a / d 键：侧滚（Roll）
		支持固定速度自动前行，使用 + / - 键调节飞行速度。
	    支持第一人称视角与第三人称追尾视角切换，
 	物体绘制
   		在场景中绘制飞机模型 enterprise.obj，
		在天空中绘制两个球体作为太阳和月亮：
	光照系统
		白天模式下模拟太阳光，强度较高，颜色偏暖；夜晚模式下模拟月亮光，强度较低，颜色偏冷；
		可通过按键交互切换昼夜模式。
		聚光灯（Spot Light），作为飞机上的探照灯，
	平面明暗与平滑明暗
   		平面明暗（Flat Shading）：每个三角形使用一个面法向；
   		平滑明暗（Smooth Shading）：每个顶点使用平均法向；
   		可通过按键在两种方式间切换。
	碰撞检测
   		实现简单的碰撞检测，飞机不能进入地形内部或地下；
二、程序运行环境
	  Windows 11 64位操作系统
	  Visual Studio 2022
	  OpenGL
	  GLM
	  FreeGLUT
三、文件结构
Project1
　　　　├─ Camera.h
　　　　├─ Materials.h
　　　　├─ Math3D.h
　　　　├─ Mesh.h
　　　　├─ ObjLoader.h
　　　　├─ Primitives.h
　　　　├─ resource.h
　　　　├─ Shader.h
　　　　├─ Terrain.h
　　　　├─ Texture.h
　　　　├─ Camera.cpp
　　　　├─ main.cpp
　　　　├─ Mesh.cpp
　　　　├─ ObjLoader.cpp
　　　　├─ Shader.cpp
　　　　├─ Terrain.cpp
　　　　├─ Texture.cpp
　　　　├─ phong.frag
　　　　├─ phong.vert
　　　　└─ readme.txt
五、操作说明
	飞机姿态控制
		← / → ：偏航（Yaw）
		↑ / ↓ ：俯仰（Pitch）
		a / d ：侧滚（Roll）
	飞行与速度
		Space ：自动前进 开 / 关
		+ ：加速
		 - ：减速
	光照与环境
		t ：白天 / 夜晚切换
		c ：碰撞检测 开 / 关
	明暗处理
		1 ：平面明暗（Flat Shading）
		2 ：平滑明暗（Smooth Shading）
	视角
		v ：第一人称 / 第三人称视角切换
		r ：重置相机
		ESC ：退出程序
六、关键实现技术说明
	所有几何变换、光照计算与纹理映射均在 GLSL 着色器中完成。
	使用 Phong 光照模型，实现多光源叠加。
	太阳与月亮球体采用 Unlit 模式直接输出颜色，避免受光照影响导致颜色相同。
	飞机模型通过相机姿态矩阵绑定，保证旋转与移动的一致性。

