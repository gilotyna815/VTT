/**
 * Code and comments based on Ryan Schmidt's UE5RuntimeToolsFrameworkDemo obtained on MIT license
 * https://github.com/gradientspace/UE5RuntimeToolsFrameworkDemo
 *
 * MIT License
 *
 * Copyright (c) 2021 gradientspace
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GeneratedMeshPool.generated.h"

/**
 * UGeneratedMeshPool manages a Pool of UGeneratedMesh objects. This allows the meshes to be re-used instead of being garbage collected.
 * 
 * Usage is to call RequestMesh() to take ownership of an available UGeneratedMesh (which will allocate a new one if the pool is empty) and ReturnMesh() to return it to the pool.
 * 
 * ReturnAllMeshes() can be called to return all allocated meshes.
 * 
 * In both cases, there is nmothing preventing you from still holding on to the mesh, so, be careful.
 * 
 * FreeAllMeshes() calls ReturnsAllMeshes() and then releases the pool's reference to the allocated meshes, so they can be GTarbage Collected.
 * 
 * If you Request() more meshes than you Return(), the Pool will still be holding on to references to thoser meshes, and they will never be Garbage Collected (ie memory leak).
 * As a failsafe, if the number of allocated meshes exceeds MeshCountSafetyThreshold, the Pool will release all it's references and run garbage collection on the next call to RequestMesh().
 * (Do not rely on this a memory management strategy)
 * 
 * An alternative strategy that could be employed here is for the Pool to not hold references to meshes it has provided, only those that have been explicitly returned.
 * Then non-returned meshes would simply be garbage-collected, however it allows potentially a large amount of memory to be consumed until that occurs.
 * 
 * UGeneratedMesh::ResetMesh() is called on the object returned to the Pool, which clears the internal FDynamicMesh3 (which uses normal C++ memory management, so no garbage collection involved.
 * So the Pool does not reuse mesh memory, only the UObject containers.
 */
UCLASS(Transient)
class VTT_API UGeneratedMeshPool : public UObject
{
	GENERATED_BODY()
	
};
