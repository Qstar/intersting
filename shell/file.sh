#!/bin/bash
#Program:
#    Check same line in different files
#    Usage:command [选项] ... 目录名
#    选项：
#    -p:        print which line have been included by which files
#    -e [文件名]:    extract duplicated line to a specified file
#    -r:        remove duplicated line in the file which include it
#History:
#2011/10/26    gaodayue    First release
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin
export PATH


usage="Usage: $0 -[per]  DirectoryName"
workingdir=$(pwd) # 当前的目录名
directoryname="" # 检索的目录名
temp_file=${workingdir}"/.files" # 存放检索目录下的文件名
sameline_file=${workingdir}"/.sameline" # 存放相同行的文件
havesameline_file=${workingdir}"/.havingsameline" # 存放拥有相同行文件名的文件
touch $temp_file
touch $sameline_file
touch $havesameline_file

# check if $1 is a directory and can read
check_directory(){
    # if $1 is a directory?
    [ ! -d $1 ] && echo "ERROR: '$1' is not an directory!" && exit 1
    # can we enter the directory?
    [ ! -r $1 -o ! -x $1 ] && echo "ERROR: DO NOT have the Permission to enter the '$1' directory!" && exit 1
    directoryname=$1
}


parse_same_line(){
    cd $directoryname
    # 得到目录下的文件名
    files=$(ls -l | awk '$8!="" && $1~/^-/{printf $8 ""} ')
    # 将文件名保存到.files文件下
    echo $files > $temp_file
    # 得到目录下文件的个数
    file_num=$(cat $temp_file | wc -w)

    # 对这些文件进行两两比较相同的行
    # 相同的行存入.sameline文件中
    # 拥有相同行的文件名存入.havesameline文件的对应行中
    # 比如：
    # .sameline文件：
    #    line1
    #    line2
    # .havesameline文件：
    #    1.txt 2.txt
    #    2.txt 3.txt
    # 表示
    #    1.txt和2.txt拥有相同行line1
    #    2.txt和3.txt拥有相同行line2

    # 两两循环比较
    i=1
    while [ $i -lt $file_num ]
    do
        file1=$(cat $temp_file | cut -d ' ' -f $i)
        j=$(($i+1))
        while [ $j -le $file_num ]
        do
            file2=$(cat $temp_file | cut -d ' ' -f $j)
            samelines=$(grep -Fxf $file1 $file2) # 得到两个文件所有的相同行
            IFS_OLD=$IFS
            IFS="
"
            for sameline in $samelines
            do
                $(grep -xq "$sameline" $sameline_file) # 在.sameline文件中查找该行
                if [ $? -eq 0 ];then
                    # 若.sameline文件中已经有该行
                    # 得到该行的行数
                    linenum=$(grep -xn "$sameline" $sameline_file | sed 's/:.*//')
                    # 取得.havesameline文件中对应行数的内容,即包含该行的文件
                    oldcontent=$(cat $havesameline_file | head -n $linenum | tail -n 1)
                    $(echo $oldcontent | grep -wq "$file2") # 查找包含该相同行的文件名中有没有file2
                    if [ $? -ne 0 ];then
                        # 若包含该行的文件名中还没有file2，则加入file2
                        newcontent=${oldcontent}' '$file2
                        $(sed -i "${linenum}s/.*/${newcontent}/" $havesameline_file)
                    fi
                else
                    # 若.sameline文件中还没有该行
                    # 增加该行，并在.havesameline文件中添加这两个文件名
                    echo $sameline >> $sameline_file
                    echo ${file1}' '${file2} >> $havesameline_file
                fi
                        
            done
            IFS=$IFS_OLD
            j=$(($j+1))
        done
        i=$(($i+1))
    done
    # 回到工作目录
    cd $workingdir
    # 删除用于临时存储文件名的.files文件
    $(rm $temp_file)
}

# 解析完成后，删除.sameline与.havingsameline文件
# 该函数与parse_same_line成对出现
end_parse(){
    $(rm $sameline_file)
    $(rm $havesameline_file)
}

# -p: print same lines
printsameline(){
    # 解析相同行
    parse_same_line
    # 如果没有相同行，提示信息并返回
    [ $(cat "${sameline_file}" | wc -l) -eq 0 ] && echo "No same line Found!" && return 0
    # 否则，输出结果
    linenum=1
    while read line
    do
        # 根据行号得到有这行的文件名
        thefiles=$(cat "${havesameline_file}" | head -n $linenum | tail -n 1)
        # 文件数
        thefilesnum=$(echo $thefiles | wc -w)
        # 输出
        echo "${line}\nis included in ${thefilesnum} files"
        for item in $thefiles
        do
            echo "${item}"
        done
        echo ""
        linenum=$(($linenum+1))
    done < "$sameline_file"
    # 结束解析
    end_parse
}

# -e [文件名]: extract duplicated line to a specified file
extract(){
    # $1为指定要输出的文件
    if [ -e $1 ];then
        # $1存在，但不是文件，提示出错
        [ ! -f $1 ] && echo "ERROR: '$1' EXISTS but IS NOT a file" && return 1
        # $1存在，且为文件，让用户选择是否继续？
        while [ "${isContinue}" != 'y' -a "${isContinue}" != 'n' ]
        do
            read -p "'$1' exists, continue will replace its content.Continue?(y/n):" isContinue
        done
        if [ "${isContinue}" = 'y' ];then
            # 判断对该文件是否有写权限
            [ ! -w $1 ] && echo "DO NOT have the WRITE PERMISSION to file '$1'" && return 1
            # 有写权限，则将-p的执行结果写入文件
            printsameline > $1
        else
            return 0
        fi
    else
        # $1不存在,则创建
        touch $1 2> /dev/null
        # 创建失败，则提示信息
        [ $? -ne 0 ] && echo "CAN NOT CREATE the file '$1'" && return 1
        # 创建成功，则将-p的执行结果写入文件
        printsameline > $1
    fi
}

# -r: remove duplicated line in the file which include it
remove(){
    # 执行解析
    parse_same_line
    # 处理解析结果
    linenum=1
    while read line
    do
        # 根据行号得到有这行的文件名
        thefiles=$(cat "${havesameline_file}" | head -n $linenum | tail -n 1)
        # 在每个文件中删除该行
        for item in $thefiles
        do
            item_fullname="${directoryname}/${item}"
            # 删除item_fullname文件中line这行
            $(sed -i /"^${line}$"/d ${item_fullname})
        done
        linenum=$(($linenum+1))
    done < "$sameline_file"
    # 结束解析
    end_parse
}

# 检查参数
case $1 in
"-p")
    [ $# -ne 2 ] && echo $usage && exit 1
    check_directory $2
    printsameline
    ;;
"-e")
    [ $# -ne 3 ] && echo $usage && exit 1
    check_directory $3
    extract $2
    ;;
"-r")
    [ $# -ne 2 ] && echo $usage && exit 1
    check_directory $2
    remove    
    ;;
*)
    echo $usage;exit 1
    ;;
esac

exit $?
