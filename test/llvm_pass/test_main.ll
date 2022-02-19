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
@stdout = external global %struct._IO_FILE*, align 8
@.str.1 = private unnamed_addr constant [18 x i8] c"String too long!\0A\00", align 1
@.str.2 = private unnamed_addr constant [36 x i8] c"Buffer overflow on stack occurred.\0A\00", align 1
@__const.main.characters = private unnamed_addr constant [3 x i8] c"xyz", align 1

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
  store i8* %0, i8** %2, align 8
  %3 = load i8*, i8** %2, align 8
  %4 = load i8, i8* %3, align 1
  %5 = sext i8 %4 to i32
  %6 = load i8*, i8** %2, align 8
  %7 = getelementptr inbounds i8, i8* %6, i64 1
  %8 = load i8, i8* %7, align 1
  %9 = sext i8 %8 to i32
  %10 = load i8*, i8** %2, align 8
  %11 = getelementptr inbounds i8, i8* %10, i64 2
  %12 = load i8, i8* %11, align 1
  %13 = sext i8 %12 to i32
  %14 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), i32 %5, i32 %9, i32 %13)
  ret void
}

declare i32 @printf(i8*, ...) #2

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

; Function Attrs: nounwind readonly willreturn
declare i64 @strlen(i8*) #3

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
  br label %16, !llvm.loop !6

34:                                               ; preds = %16
  %35 = load i32, i32* %5, align 4
  %36 = icmp sgt i32 %35, 36
  br i1 %36, label %37, label %41

37:                                               ; preds = %34
  %38 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %39 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %38, i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str.1, i64 0, i64 0))
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
  %52 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %51, i8* getelementptr inbounds ([36 x i8], [36 x i8]* @.str.2, i64 0, i64 0))
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
  %7 = alloca %struct.A*, align 8
  %8 = alloca i32*, align 8
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  store i8** %1, i8*** %5, align 8
  %9 = load i32, i32* %4, align 4
  %10 = icmp ne i32 %9, 2
  br i1 %10, label %11, label %12

11:                                               ; preds = %2
  call void @exit(i32 1) #8
  unreachable

12:                                               ; preds = %2
  %13 = bitcast [3 x i8]* %6 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %13, i8* align 1 getelementptr inbounds ([3 x i8], [3 x i8]* @__const.main.characters, i32 0, i32 0), i64 3, i1 false)
  %14 = getelementptr inbounds [3 x i8], [3 x i8]* %6, i64 0, i64 0
  call void @print_characters(i8* %14)
  %15 = call %struct.A* @initialize_struct()
  store %struct.A* %15, %struct.A** %7, align 8
  %16 = call i32* @initialize_array(i32 10)
  store i32* %16, i32** %8, align 8
  %17 = load %struct.A*, %struct.A** %7, align 8
  %18 = bitcast %struct.A* %17 to i8*
  call void @free(i8* %18) #6
  %19 = load i32*, i32** %8, align 8
  %20 = bitcast i32* %19 to i8*
  call void @free(i8* %20) #6
  %21 = load i8**, i8*** %5, align 8
  %22 = getelementptr inbounds i8*, i8** %21, i64 1
  %23 = load i8*, i8** %22, align 8
  %24 = call i32 @un_init(i8* %23)
  ret i32 0
}

; Function Attrs: noreturn nounwind
declare void @exit(i32) #4

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #5

; Function Attrs: nounwind
declare void @free(i8*) #1

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
