; ModuleID = 'test_main.c'
source_filename = "test_main.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._IO_codecvt*, %struct._IO_wide_data*, %struct._IO_FILE*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type opaque
%struct._IO_codecvt = type opaque
%struct._IO_wide_data = type opaque
%struct.A = type { %struct.B, i8, i32*, i16, float, double, i8, i64, i32 }
%struct.B = type { i64*, i8**, i8, %struct.C }
%struct.C = type { i8 }

@GLOBAL_VARIABLE = dso_local global i32 666, align 4
@.str = private unnamed_addr constant [8 x i8] c"%c%c%c\0A\00", align 1
@.str.1 = private unnamed_addr constant [39 x i8] c"user string too long, die evil hacker!\00", align 1
@.str.2 = private unnamed_addr constant [5 x i8] c"boat\00", align 1
@.str.3 = private unnamed_addr constant [4 x i8] c"car\00", align 1
@.str.4 = private unnamed_addr constant [6 x i8] c"truck\00", align 1
@.str.5 = private unnamed_addr constant [6 x i8] c"train\00", align 1
@__const.BufferOverRead.items = private unnamed_addr constant [4 x i8*] [i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.2, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.4, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.5, i32 0, i32 0)], align 16
@.str.6 = private unnamed_addr constant [17 x i8] c"You selected %s\0A\00", align 1
@stdout = external global %struct._IO_FILE*, align 8
@.str.7 = private unnamed_addr constant [18 x i8] c"String too long!\0A\00", align 1
@.str.8 = private unnamed_addr constant [36 x i8] c"Buffer overflow on stack occurred.\0A\00", align 1
@__const.main.characters = private unnamed_addr constant [3 x i8] c"xyz", align 1
@.str.9 = private unnamed_addr constant [10 x i8] c"&&&&&&&&&\00", align 1
@.str.10 = private unnamed_addr constant [2 x i8] c"/\00", align 1

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local %struct.A* @initialize_struct() #0 {
  %1 = alloca %struct.A*, align 8
  %2 = call noalias align 16 i8* @malloc(i64 80) #6
  %3 = bitcast i8* %2 to %struct.A*
  store %struct.A* %3, %struct.A** %1, align 8
  %4 = load %struct.A*, %struct.A** %1, align 8
  %5 = getelementptr inbounds %struct.A, %struct.A* %4, i32 0, i32 0
  %6 = getelementptr inbounds %struct.B, %struct.B* %5, i32 0, i32 0
  store i64* null, i64** %6, align 8
  %7 = load %struct.A*, %struct.A** %1, align 8
  %8 = getelementptr inbounds %struct.A, %struct.A* %7, i32 0, i32 0
  %9 = getelementptr inbounds %struct.B, %struct.B* %8, i32 0, i32 1
  store i8** null, i8*** %9, align 8
  %10 = load %struct.A*, %struct.A** %1, align 8
  %11 = getelementptr inbounds %struct.A, %struct.A* %10, i32 0, i32 0
  %12 = getelementptr inbounds %struct.B, %struct.B* %11, i32 0, i32 2
  store i8 0, i8* %12, align 8
  %13 = load %struct.A*, %struct.A** %1, align 8
  %14 = getelementptr inbounds %struct.A, %struct.A* %13, i32 0, i32 8
  store i32 228, i32* %14, align 8
  %15 = load %struct.A*, %struct.A** %1, align 8
  %16 = getelementptr inbounds %struct.A, %struct.A* %15, i32 0, i32 1
  store i8 97, i8* %16, align 8
  %17 = load %struct.A*, %struct.A** %1, align 8
  %18 = getelementptr inbounds %struct.A, %struct.A* %17, i32 0, i32 8
  %19 = load %struct.A*, %struct.A** %1, align 8
  %20 = getelementptr inbounds %struct.A, %struct.A* %19, i32 0, i32 2
  store i32* %18, i32** %20, align 8
  %21 = load %struct.A*, %struct.A** %1, align 8
  %22 = getelementptr inbounds %struct.A, %struct.A* %21, i32 0, i32 3
  store i16 1000, i16* %22, align 8
  %23 = load %struct.A*, %struct.A** %1, align 8
  %24 = getelementptr inbounds %struct.A, %struct.A* %23, i32 0, i32 4
  store float 0x40091EB860000000, float* %24, align 4
  %25 = load %struct.A*, %struct.A** %1, align 8
  %26 = getelementptr inbounds %struct.A, %struct.A* %25, i32 0, i32 5
  store double 2.900000e+00, double* %26, align 8
  %27 = load %struct.A*, %struct.A** %1, align 8
  %28 = getelementptr inbounds %struct.A, %struct.A* %27, i32 0, i32 6
  store i8 1, i8* %28, align 8
  %29 = load %struct.A*, %struct.A** %1, align 8
  %30 = getelementptr inbounds %struct.A, %struct.A* %29, i32 0, i32 7
  store i64 5, i64* %30, align 8
  %31 = load %struct.A*, %struct.A** %1, align 8
  %32 = getelementptr inbounds %struct.A, %struct.A* %31, i32 0, i32 0
  %33 = getelementptr inbounds %struct.B, %struct.B* %32, i32 0, i32 3
  %34 = getelementptr inbounds %struct.C, %struct.C* %33, i32 0, i32 0
  store i8 98, i8* %34, align 1
  %35 = load %struct.A*, %struct.A** %1, align 8
  ret %struct.A* %35
}

; Function Attrs: nounwind
declare noalias align 16 i8* @malloc(i64) #1

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local i32* @initialize_array(i32 %0) #0 {
  %2 = alloca i32*, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32*, align 8
  store i32 %0, i32* %3, align 4
  %5 = load i32, i32* %3, align 4
  %6 = icmp slt i32 %5, 1
  br i1 %6, label %7, label %8

7:                                                ; preds = %1
  store i32* null, i32** %2, align 8
  br label %14

8:                                                ; preds = %1
  %9 = load i32, i32* %3, align 4
  %10 = sext i32 %9 to i64
  %11 = call noalias align 16 i8* @malloc(i64 %10) #6
  %12 = bitcast i8* %11 to i32*
  store i32* %12, i32** %4, align 8
  %13 = load i32*, i32** %4, align 8
  store i32* %13, i32** %2, align 8
  br label %14

14:                                               ; preds = %8, %7
  %15 = load i32*, i32** %2, align 8
  ret i32* %15
}

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local void @print_characters(i8* %0) #0 {
  %2 = alloca i8*, align 8
  %3 = alloca %struct.A*, align 8
  %4 = alloca i32*, align 8
  store i8* %0, i8** %2, align 8
  %5 = call %struct.A* @initialize_struct()
  store %struct.A* %5, %struct.A** %3, align 8
  %6 = call i32* @initialize_array(i32 10)
  store i32* %6, i32** %4, align 8
  %7 = load %struct.A*, %struct.A** %3, align 8
  %8 = bitcast %struct.A* %7 to i8*
  call void @free(i8* %8) #6
  %9 = load i32*, i32** %4, align 8
  %10 = bitcast i32* %9 to i8*
  call void @free(i8* %10) #6
  %11 = load i8*, i8** %2, align 8
  %12 = load i8, i8* %11, align 1
  %13 = sext i8 %12 to i32
  %14 = load i8*, i8** %2, align 8
  %15 = getelementptr inbounds i8, i8* %14, i64 1
  %16 = load i8, i8* %15, align 1
  %17 = sext i8 %16 to i32
  %18 = load i8*, i8** %2, align 8
  %19 = getelementptr inbounds i8, i8* %18, i64 2
  %20 = load i8, i8* %19, align 1
  %21 = sext i8 %20 to i32
  %22 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), i32 %13, i32 %17, i32 %21)
  ret void
}

; Function Attrs: nounwind
declare void @free(i8*) #1

declare i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local i8* @copy_input(i8* %0) #0 {
  %2 = alloca i8*, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8*, align 8
  store i8* %0, i8** %2, align 8
  %6 = call noalias align 16 i8* @malloc(i64 40) #6
  store i8* %6, i8** %5, align 8
  %7 = load i8*, i8** %2, align 8
  %8 = call i64 @strlen(i8* %7) #7
  %9 = icmp ule i64 10, %8
  br i1 %9, label %10, label %12

10:                                               ; preds = %1
  %11 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([39 x i8], [39 x i8]* @.str.1, i64 0, i64 0))
  call void @exit(i32 1) #8
  unreachable

12:                                               ; preds = %1
  store i32 0, i32* %4, align 4
  store i32 0, i32* %3, align 4
  br label %13

13:                                               ; preds = %75, %12
  %14 = load i32, i32* %3, align 4
  %15 = sext i32 %14 to i64
  %16 = load i8*, i8** %2, align 8
  %17 = call i64 @strlen(i8* %16) #7
  %18 = icmp ult i64 %15, %17
  br i1 %18, label %19, label %78

19:                                               ; preds = %13
  %20 = load i8*, i8** %2, align 8
  %21 = load i32, i32* %3, align 4
  %22 = sext i32 %21 to i64
  %23 = getelementptr inbounds i8, i8* %20, i64 %22
  %24 = load i8, i8* %23, align 1
  %25 = sext i8 %24 to i32
  %26 = icmp eq i32 38, %25
  br i1 %26, label %27, label %53

27:                                               ; preds = %19
  %28 = load i8*, i8** %5, align 8
  %29 = load i32, i32* %4, align 4
  %30 = add nsw i32 %29, 1
  store i32 %30, i32* %4, align 4
  %31 = sext i32 %29 to i64
  %32 = getelementptr inbounds i8, i8* %28, i64 %31
  store i8 38, i8* %32, align 1
  %33 = load i8*, i8** %5, align 8
  %34 = load i32, i32* %4, align 4
  %35 = add nsw i32 %34, 1
  store i32 %35, i32* %4, align 4
  %36 = sext i32 %34 to i64
  %37 = getelementptr inbounds i8, i8* %33, i64 %36
  store i8 97, i8* %37, align 1
  %38 = load i8*, i8** %5, align 8
  %39 = load i32, i32* %4, align 4
  %40 = add nsw i32 %39, 1
  store i32 %40, i32* %4, align 4
  %41 = sext i32 %39 to i64
  %42 = getelementptr inbounds i8, i8* %38, i64 %41
  store i8 109, i8* %42, align 1
  %43 = load i8*, i8** %5, align 8
  %44 = load i32, i32* %4, align 4
  %45 = add nsw i32 %44, 1
  store i32 %45, i32* %4, align 4
  %46 = sext i32 %44 to i64
  %47 = getelementptr inbounds i8, i8* %43, i64 %46
  store i8 112, i8* %47, align 1
  %48 = load i8*, i8** %5, align 8
  %49 = load i32, i32* %4, align 4
  %50 = add nsw i32 %49, 1
  store i32 %50, i32* %4, align 4
  %51 = sext i32 %49 to i64
  %52 = getelementptr inbounds i8, i8* %48, i64 %51
  store i8 59, i8* %52, align 1
  br label %74

53:                                               ; preds = %19
  %54 = load i8*, i8** %2, align 8
  %55 = load i32, i32* %3, align 4
  %56 = sext i32 %55 to i64
  %57 = getelementptr inbounds i8, i8* %54, i64 %56
  %58 = load i8, i8* %57, align 1
  %59 = sext i8 %58 to i32
  %60 = icmp eq i32 60, %59
  br i1 %60, label %61, label %62

61:                                               ; preds = %53
  br label %73

62:                                               ; preds = %53
  %63 = load i8*, i8** %2, align 8
  %64 = load i32, i32* %3, align 4
  %65 = sext i32 %64 to i64
  %66 = getelementptr inbounds i8, i8* %63, i64 %65
  %67 = load i8, i8* %66, align 1
  %68 = load i8*, i8** %5, align 8
  %69 = load i32, i32* %4, align 4
  %70 = add nsw i32 %69, 1
  store i32 %70, i32* %4, align 4
  %71 = sext i32 %69 to i64
  %72 = getelementptr inbounds i8, i8* %68, i64 %71
  store i8 %67, i8* %72, align 1
  br label %73

73:                                               ; preds = %62, %61
  br label %74

74:                                               ; preds = %73, %27
  br label %75

75:                                               ; preds = %74
  %76 = load i32, i32* %3, align 4
  %77 = add nsw i32 %76, 1
  store i32 %77, i32* %3, align 4
  br label %13, !llvm.loop !6

78:                                               ; preds = %13
  %79 = load i8*, i8** %5, align 8
  ret i8* %79
}

; Function Attrs: nounwind readonly willreturn
declare i64 @strlen(i8*) #3

; Function Attrs: noreturn nounwind
declare void @exit(i32) #4

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local void @BufferOverRead() #0 {
  %1 = alloca [4 x i8*], align 16
  %2 = alloca i32, align 4
  %3 = bitcast [4 x i8*]* %1 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %3, i8* align 16 bitcast ([4 x i8*]* @__const.BufferOverRead.items to i8*), i64 32, i1 false)
  store i32 10, i32* %2, align 4
  %4 = load i32, i32* %2, align 4
  %5 = sub nsw i32 %4, 1
  %6 = sext i32 %5 to i64
  %7 = getelementptr inbounds [4 x i8*], [4 x i8*]* %1, i64 0, i64 %6
  %8 = load i8*, i8** %7, align 8
  %9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str.6, i64 0, i64 0), i8* %8)
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #5

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local zeroext i1 @is_valid(i8* %0) #0 {
  %2 = alloca i1, align 1
  %3 = alloca i8*, align 8
  store i8* %0, i8** %3, align 8
  %4 = load i8*, i8** %3, align 8
  %5 = icmp ne i8* %4, null
  br i1 %5, label %6, label %7

6:                                                ; preds = %1
  store i1 true, i1* %2, align 1
  br label %8

7:                                                ; preds = %1
  store i1 false, i1* %2, align 1
  br label %8

8:                                                ; preds = %7, %6
  %9 = load i1, i1* %2, align 1
  ret i1 %9
}

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local i32 @get_line_len(i8* %0) #0 {
  %2 = alloca i8*, align 8
  store i8* %0, i8** %2, align 8
  %3 = load i8*, i8** %2, align 8
  %4 = call i64 @strlen(i8* %3) #7
  %5 = trunc i64 %4 to i32
  ret i32 %5
}

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local i32 @un_init(i8* %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca i8*, align 8
  %4 = alloca [32 x i8], align 16
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store i8* %0, i8** %3, align 8
  store i32 0, i32* %5, align 4
  store i32 0, i32* %6, align 4
  %9 = load i8*, i8** %3, align 8
  %10 = call zeroext i1 @is_valid(i8* %9)
  br i1 %10, label %13, label %11

11:                                               ; preds = %1
  %12 = load i32, i32* %6, align 4
  store i32 %12, i32* %2, align 4
  br label %55

13:                                               ; preds = %1
  %14 = load i8*, i8** %3, align 8
  %15 = call i32 @get_line_len(i8* %14)
  store i32 %15, i32* %5, align 4
  store i32 0, i32* %7, align 4
  store i32 0, i32* %8, align 4
  br label %16

16:                                               ; preds = %31, %13
  %17 = load i32, i32* %8, align 4
  %18 = load i32, i32* %5, align 4
  %19 = icmp slt i32 %17, %18
  br i1 %19, label %20, label %34

20:                                               ; preds = %16
  %21 = load i32, i32* %8, align 4
  %22 = load i32, i32* %7, align 4
  %23 = add nsw i32 %22, %21
  store i32 %23, i32* %7, align 4
  %24 = load i32, i32* %7, align 4
  %25 = srem i32 %24, 2
  %26 = icmp eq i32 %25, 0
  br i1 %26, label %27, label %30

27:                                               ; preds = %20
  %28 = load i32, i32* %7, align 4
  %29 = sdiv i32 %28, 2
  store i32 %29, i32* %7, align 4
  br label %30

30:                                               ; preds = %27, %20
  br label %31

31:                                               ; preds = %30
  %32 = load i32, i32* %8, align 4
  %33 = add nsw i32 %32, 1
  store i32 %33, i32* %8, align 4
  br label %16, !llvm.loop !8

34:                                               ; preds = %16
  %35 = load i32, i32* %5, align 4
  %36 = icmp sgt i32 %35, 36
  br i1 %36, label %37, label %41

37:                                               ; preds = %34
  %38 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %39 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %38, i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str.7, i64 0, i64 0))
  %40 = load i32, i32* %6, align 4
  store i32 %40, i32* %2, align 4
  br label %55

41:                                               ; preds = %34
  %42 = getelementptr inbounds [32 x i8], [32 x i8]* %4, i64 0, i64 0
  %43 = load i8*, i8** %3, align 8
  %44 = load i32, i32* %5, align 4
  %45 = sext i32 %44 to i64
  %46 = call i8* @strncpy(i8* %42, i8* %43, i64 %45) #6
  br label %47

47:                                               ; preds = %41
  %48 = load i32, i32* %6, align 4
  %49 = icmp eq i32 %48, 1094795585
  br i1 %49, label %50, label %53

50:                                               ; preds = %47
  %51 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %52 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %51, i8* getelementptr inbounds ([36 x i8], [36 x i8]* @.str.8, i64 0, i64 0))
  br label %53

53:                                               ; preds = %50, %47
  store i32 1, i32* %6, align 4
  %54 = load i32, i32* %6, align 4
  store i32 %54, i32* %2, align 4
  br label %55

55:                                               ; preds = %53, %37, %11
  %56 = load i32, i32* %2, align 4
  ret i32 %56
}

declare i32 @fprintf(%struct._IO_FILE*, i8*, ...) #2

; Function Attrs: nounwind
declare i8* @strncpy(i8*, i8*, i64) #1

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local i32 @main(i32 %0, i8** %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca [3 x i8], align 1
  %7 = alloca i8*, align 8
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  store i8** %1, i8*** %5, align 8
  %8 = load i32, i32* %4, align 4
  %9 = icmp ne i32 %8, 2
  br i1 %9, label %10, label %11

10:                                               ; preds = %2
  call void @exit(i32 1) #8
  unreachable

11:                                               ; preds = %2
  %12 = bitcast [3 x i8]* %6 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %12, i8* align 1 getelementptr inbounds ([3 x i8], [3 x i8]* @__const.main.characters, i32 0, i32 0), i64 3, i1 false)
  %13 = getelementptr inbounds [3 x i8], [3 x i8]* %6, i64 0, i64 0
  call void @print_characters(i8* %13)
  store i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.9, i64 0, i64 0), i8** %7, align 8
  %14 = load i8*, i8** %7, align 8
  %15 = call i8* @copy_input(i8* %14)
  call void @BufferOverRead()
  %16 = load i8**, i8*** %5, align 8
  %17 = getelementptr inbounds i8*, i8** %16, i64 1
  %18 = load i8*, i8** %17, align 8
  %19 = call i32 @un_init(i8* %18)
  %20 = call i8* @sanitize_cookie_path(i8* inttoptr (i64 34 to i8*))
  ret i32 0
}

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define internal i8* @sanitize_cookie_path(i8* %0) #0 {
  %2 = alloca i8*, align 8
  %3 = alloca i8*, align 8
  %4 = alloca i64, align 8
  %5 = alloca i8*, align 8
  store i8* %0, i8** %3, align 8
  %6 = load i8*, i8** %3, align 8
  %7 = call noalias align 16 i8* @strdup(i8* %6) #6
  store i8* %7, i8** %5, align 8
  %8 = load i8*, i8** %5, align 8
  %9 = icmp ne i8* %8, null
  br i1 %9, label %11, label %10

10:                                               ; preds = %1
  store i8* null, i8** %2, align 8
  br label %68

11:                                               ; preds = %1
  %12 = load i8*, i8** %5, align 8
  %13 = getelementptr inbounds i8, i8* %12, i64 0
  %14 = load i8, i8* %13, align 1
  %15 = sext i8 %14 to i32
  %16 = icmp eq i32 %15, 34
  br i1 %16, label %17, label %23

17:                                               ; preds = %11
  %18 = load i8*, i8** %5, align 8
  %19 = load i8*, i8** %5, align 8
  %20 = getelementptr inbounds i8, i8* %19, i64 1
  %21 = load i8*, i8** %5, align 8
  %22 = call i64 @strlen(i8* %21) #7
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 1 %18, i8* align 1 %20, i64 %22, i1 false)
  br label %23

23:                                               ; preds = %17, %11
  %24 = load i8*, i8** %5, align 8
  %25 = load i8*, i8** %5, align 8
  %26 = call i64 @strlen(i8* %25) #7
  %27 = sub i64 %26, 1
  %28 = getelementptr inbounds i8, i8* %24, i64 %27
  %29 = load i8, i8* %28, align 1
  %30 = sext i8 %29 to i32
  %31 = icmp eq i32 %30, 34
  br i1 %31, label %32, label %38

32:                                               ; preds = %23
  %33 = load i8*, i8** %5, align 8
  %34 = load i8*, i8** %5, align 8
  %35 = call i64 @strlen(i8* %34) #7
  %36 = sub i64 %35, 1
  %37 = getelementptr inbounds i8, i8* %33, i64 %36
  store i8 0, i8* %37, align 1
  br label %38

38:                                               ; preds = %32, %23
  %39 = load i8*, i8** %5, align 8
  %40 = getelementptr inbounds i8, i8* %39, i64 0
  %41 = load i8, i8* %40, align 1
  %42 = sext i8 %41 to i32
  %43 = icmp ne i32 %42, 47
  br i1 %43, label %44, label %48

44:                                               ; preds = %38
  %45 = load i8*, i8** %5, align 8
  call void @free(i8* %45) #6
  %46 = call noalias align 16 i8* @strdup(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.10, i64 0, i64 0)) #6
  store i8* %46, i8** %5, align 8
  %47 = load i8*, i8** %5, align 8
  store i8* %47, i8** %2, align 8
  br label %68

48:                                               ; preds = %38
  %49 = load i8*, i8** %5, align 8
  %50 = call i64 @strlen(i8* %49) #7
  store i64 %50, i64* %4, align 8
  %51 = load i64, i64* %4, align 8
  %52 = icmp ult i64 1, %51
  br i1 %52, label %53, label %66

53:                                               ; preds = %48
  %54 = load i8*, i8** %5, align 8
  %55 = load i64, i64* %4, align 8
  %56 = sub i64 %55, 1
  %57 = getelementptr inbounds i8, i8* %54, i64 %56
  %58 = load i8, i8* %57, align 1
  %59 = sext i8 %58 to i32
  %60 = icmp eq i32 %59, 47
  br i1 %60, label %61, label %66

61:                                               ; preds = %53
  %62 = load i8*, i8** %5, align 8
  %63 = load i64, i64* %4, align 8
  %64 = sub i64 %63, 1
  %65 = getelementptr inbounds i8, i8* %62, i64 %64
  store i8 0, i8* %65, align 1
  br label %66

66:                                               ; preds = %61, %53, %48
  %67 = load i8*, i8** %5, align 8
  store i8* %67, i8** %2, align 8
  br label %68

68:                                               ; preds = %66, %44, %10
  %69 = load i8*, i8** %2, align 8
  ret i8* %69
}

; Function Attrs: nounwind
declare noalias align 16 i8* @strdup(i8*) #1

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memmove.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1 immarg) #5

attributes #0 = { noinline nounwind optnone sspstrong uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind readonly willreturn "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { noreturn nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { argmemonly nofree nounwind willreturn }
attributes #6 = { nounwind }
attributes #7 = { nounwind readonly willreturn }
attributes #8 = { noreturn nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 13.0.0"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
