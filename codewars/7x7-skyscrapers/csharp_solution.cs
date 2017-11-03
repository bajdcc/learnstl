using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Test01
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    /// <summary>
    /// 主要生成全排列
    /// </summary>
    class Permutation
    {
        private int[] data = new int[7] { 1, 2, 3, 4, 5, 6, 7 };

        public List<int> Next()
        {
            if (data == null) return null;
            var ret = data.ToList();
            Calc();
            return ret;
        }

        public List<int> Data()
        {
            return data.ToList();
        }

        private void Calc()
        {
            // a[k]<a[k+1] and max(k)
            int k = -1;
            for (int i = 0; i < 6; i++)
            {
                if (data[i] < data[i + 1])
                {
                    k = i;
                }
            }
            if (k == -1)
            {
                data = null;
                return;
            }
            // j>k && a[j]>a[k] && min(a[j])
            int j = k + 1;
            int ak = data[k];
            for (int i = j + 1; i < 7; i++)
            {
                int ai = data[i];
                if (ai > ak && ai <= data[j])
                {
                    j = i;
                }
            }
            // a[j] <-> a[k]
            int tmp = data[j];
            data[j] = ak;
            data[k] = tmp;
            // reverse a[k+1..end]
            for (int i = 0; i <= (6 - k) / 2 - 1; i++)
            {
                tmp = data[k + i + 1];
                data[k + i + 1] = data[6 - i];
                data[6 - i] = tmp;
            }
        }
    }

    public class Skyscrapers
    {
        private static List<Tuple<int, int>> clue; // 去除零值的表
        private static Dictionary<int, List<List<int>>> arr = new Dictionary<int, List<List<int>>>();// 缓存表

        /// <summary>
        /// 生成一侧能看到的楼房数
        /// </summary>
        /// <param name="l">排列</param>
        /// <returns></returns>
        private static int SkyLeft(List<int> l)
        {
            var a = l[0]; var b = 1;
            for (int i = 1; i < 7; i++)
            {
                if (l[i] > a)
                {
                    a = l[i]; b++;
                }
            }
            return b;
        }
        
        public static int[][] SolvePuzzle(int[] clues)
        {
            if (arr.Count == 0)
            {
                Permutation p = new Permutation();
                var l = p.Data();
                while (l != null)
                {
                    l.Reverse();//答案主要是降序较多，所以取巧了
                    var l1 = SkyLeft(l);
                    //Console.WriteLine(string.Join(",", l) + ", " + l1);
                    if (!arr.ContainsKey(l1))
                        arr.Add(l1, new List<List<int>>());
                    arr[l1].Add(l);//arr= map<SkyLeft值, 排列>
                    l = p.Next();
                }
            }
            var answer = new List<int>(49);
            clue = new List<Tuple<int, int>>();
            var cnt = 0;
            var rei = new int[28] { 20, 19, 18, 17, 16, 15, 14, 27, 26, 25, 24, 23, 22, 21, 6, 5, 4, 3, 2, 1, 0, 13, 12, 11, 10, 9, 8, 7 }; //生成该点的对边位置（街道的两侧）
            for (int i = 0; i < clues.Length; i++)
            {
                if (clues[i] != 0)
                    clue.Add(new Tuple<int, int>(i, clues[i]));
                if (clues[i] != 0 && clues[rei[i]] != 0)
                    cnt++;
            }
            int[] so;

            //两边都有数值的称为“有效边”
            if (cnt < 6)//如果有效边数<6
            {
                //生成的决策顺序是
                //优先 = 有效边且值较大（因为可能性少）
                //次要 = 较大值
                so = new int[] { 10, 4, 7, 6, 5, 3, 2, 1 };//以可能性数量降序（优化剪枝）
                clue = (from x in clue orderby so[x.Item2] + so[clues[rei[x.Item1]]] select x).ToList();
            }
            else if(cnt < 10)//有效边增多了
            {
                //减少0边的权重，为什么这样写，试出来的
                so = new int[] { 9, 4, 7, 6, 5, 3, 2, 1 };
                clue = (from x in clue orderby 1.1 * so[x.Item2] + so[clues[rei[x.Item1]]] select x).ToList();
            }
            else
            {
                //对边很多，优先选较大值
                so = new int[] { 20, 4, 7, 6, 5, 3, 2, 1 };
                clue = (from x in clue orderby so[x.Item2] select x).ToList();
            }
            //以上的决策目标是为了减少展开数
            //举例：
            //A可能性=1000，B可能性=500，那就先展开B，再展开A
            var ret = new int[7][] { new int[7] { 0, 0, 0, 0, 0, 0, 0 }, new int[7] { 0, 0, 0, 0, 0, 0, 0 }, new int[7] { 0, 0, 0, 0, 0, 0, 0 }, new int[7] { 0, 0, 0, 0, 0, 0, 0 }, new int[7] { 0, 0, 0, 0, 0, 0, 0 }, new int[7] { 0, 0, 0, 0, 0, 0, 0 }, new int[7] { 0, 0, 0, 0, 0, 0, 0 } };
            find(0, ret);
            return ret;
        }

        /// <summary>
        /// 对所有边，我都填好了值，那么有一些格子横竖上是没有提示的，我们需要反推它
        /// </summary>
        /// <param name="map"></param>
        /// <returns></returns>
        static bool Alter(int[][] map)
        {
            //Console.WriteLine(string.Join("\n", map.ToList().Select(x => string.Join(",", x))));
            var m = new List<HashSet<int>>();//一张排除表
            for (int i = 0; i < 49; i++)
            {
                m.Add(new HashSet<int>());
            }
            var cnt = 0;
            //当前格子有值，就以横竖加上此值，作为排除
            //当当前格子已经有值时，排除表是满的
            for (int i = 0; i < 7; i++)
            {
                for (int j = 0; j < 7; j++)
                {
                    var v = map[i][j];
                    if (v != 0)
                        for (int k = 0; k < 7; k++)
                        {
                            m[i * 7 + k].Add(v);
                            m[k * 7 + j].Add(v);
                        }
                    else
                        cnt++;
                }
            }
            bool flag = true;
            while (flag)
            {
                flag = false;
                for (int i = 0; i < 49; i++)
                {
                    if (m[i].Count == 6)//当且仅当为6时，可以正确推断得出值
                    {
                        flag = true;
                        var nv = 28 - m[i].Sum();
                        var ri = i / 7;
                        var ci = i % 7;
                        map[ri][ci] = nv;
                        cnt--;
                        //Console.WriteLine($"Infer: {ri},{ci} = {nv}");
                        for (int k = 0; k < 7; k++)//更新表
                        {
                            m[ri * 7 + k].Add(nv);
                            m[k * 7 + ci].Add(nv);
                        }
                    }
                }
            }
            return cnt == 0;
        }

        static bool find(int level, int[][] map)
        {
            if (level == clue.Count)//所有线索通过
            {
                var ret = map.Select(x => x.ToArray()).ToArray();
                if (Alter(ret))//有些情况空白格子是填不出的，要舍弃这些答案
                {
                    for (int i = 0; i < 7; i++)//正确答案
                    {
                        map[i] = ret[i];
                    }
                    return true;
                }
                return false;
            }
            var c = clue[level];
            var id = c.Item1;
            var val = c.Item2;
            // Console.WriteLine("----\n\n#" + level + "\n" + string.Join("\n", map.ToList().Select(x => string.Join(",", x))));
            //以下因此懒，我分了四种情况，即上下左右
            if (id >= 0 && id <= 6)
            {
                foreach (var p in arr[val])//对当前边上一点，根据值测试所有可能性
                {
                    var d = true;
                    for (int i = 0; i < 7; i++)
                    {
                        if (!(map[i][id] == 0 || p[i] == map[i][id]))//枚举过程中不与已有值冲突
                        {
                            d = false;
                            break;
                        }
                    }
                    if (d)
                    {
                        var saved = new int[7] { map[0][id], map[1][id], map[2][id], map[3][id], map[4][id], map[5][id], map[6][id] };//保存现场
                        var failed = false;
                        for (int i = 0; i < 7 && !failed; i++)
                        {
                            for (int j = 0; j < 7 && !failed; j++)
                            {
                                if (j == id) continue;
                                if (map[i][j] == p[i]) failed = true;//保证一条线上的某值是唯一的
                            }
                        }
                        if (failed)
                            continue;
                        for (int i = 0; i < 7; i++)
                        {
                            map[i][id] = p[i];//更改现场
                        }
                        if (find(level + 1, map))//深入一层搜索，深搜
                        {
                            return true;
                        }
                        for (int i = 0; i < 7; i++)
                        {
                            map[i][id] = saved[i];//恢复现场
                        }
                    }
                }
            }
            else if (id >= 7 && id <= 13)
            {
                id -= 7;
                foreach (var p in arr[val])
                {
                    var d = true;
                    for (int i = 0; i < 7; i++)
                    {
                        if (!(map[id][i] == 0 || p[6-i] == map[id][i]))
                        {
                            d = false;
                            break;
                        }
                    }
                    if (d)
                    {
                        var saved = map[id].ToArray();
                        var failed = false;
                        for (int i = 0; i < 7 && !failed; i++)
                        {
                            if (i == id) continue;
                            for (int j = 0; j < 7 && !failed; j++)
                            {
                                if (map[i][j] == p[6 - j]) failed = true;
                            }
                        }
                        if (failed) continue;
                        for (int i = 0; i < 7; i++)
                        {
                            map[id][i] = p[6 - i];
                        }
                        if (find(level + 1, map))
                        {
                            return true;
                        }
                        for (int i = 0; i < 7; i++)
                        {
                            map[id][i] = saved[i];
                        }
                    }
                }
            }
            else if (id >= 14 && id <= 20)
            {
                id = 20 - id;
                foreach (var p in arr[val])
                {
                    var d = true;
                    for (int i = 0; i < 7; i++)
                    {
                        if (!(map[i][id] == 0 || p[6-i] == map[i][id]))
                        {
                            d = false;
                            break;
                        }
                    }
                    if (d)
                    {
                        var saved = new int[7] { map[0][id], map[1][id], map[2][id], map[3][id], map[4][id], map[5][id], map[6][id] };
                        var failed = false;
                        for (int i = 0; i < 7 && !failed; i++)
                        {
                            for (int j = 0; j < 7 && !failed; j++)
                            {
                                if (j == id) continue;
                                if (map[i][j] == p[6 - i]) failed = true;
                            }
                        }
                        if (failed) continue;
                        for (int i = 0; i < 7; i++)
                        {
                            map[i][id] = p[6 - i];
                        }
                        if (find(level + 1, map))
                        {
                            return true;
                        }
                        for (int i = 0; i < 7; i++)
                        {
                            map[i][id] = saved[i];
                        }
                    }
                }
            }
            else
            {
                id = 27 - id;
                foreach (var p in arr[val])
                {
                    var d = true;
                    for (int i = 0; i < 7; i++)
                    {
                        if (!(map[id][i] == 0 || p[i] == map[id][i]))
                        {
                            d = false;
                            break;
                        }
                    }
                    if (d)
                    {
                        var saved = map[id].ToArray();
                        var failed = false;
                        for (int i = 0; i < 7 && !failed; i++)
                        {
                            if (i == id) continue;
                            for (int j = 0; j < 7 && !failed; j++)
                            {
                                if (map[i][j] == p[j]) failed = true;
                            }
                        }
                        if (failed) continue;
                        for (int i = 0; i < 7; i++)
                        {
                            map[id][i] = p[i];
                        }
                        if (find(level + 1, map))
                        {
                            return true;
                        }
                        for (int i = 0; i < 7; i++)
                        {
                            map[id][i] = saved[i];
                        }
                    }
                }
            }
            return false;//回溯
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            Stopwatch sw = new Stopwatch();
            Stopwatch sw0 = new Stopwatch();
            sw0.Start();
            sw.Start();
            Console.WriteLine(string.Join("\r\n", Skyscrapers.SolvePuzzle(new[] { 7, 0, 0, 0, 2, 2, 3, 0, 0, 3, 0, 0, 0, 0, 3, 0, 3, 0, 0, 5, 0, 0, 0, 0, 0, 5, 0, 4 }).Select(x => string.Join(" ", x))));
            sw.Stop();
            TimeSpan ts2 = sw.Elapsed;
            Console.WriteLine($"1 总共花费{ts2.TotalMilliseconds}ms.");
            sw.Restart();
            Console.WriteLine(string.Join("\r\n", Skyscrapers.SolvePuzzle(new[] { 6, 4, 0, 2, 0, 0, 3, 0, 3, 3, 3, 0, 0, 4, 0, 5, 0, 5, 0, 2, 0, 0, 0, 0, 4, 0, 0, 3 }).Select(x => string.Join(",", x))));
            sw.Stop();
            ts2 = sw.Elapsed;
            Console.WriteLine($"2 总共花费{ts2.TotalMilliseconds}ms.");
            sw.Restart();
            Console.WriteLine(string.Join("\r\n", Skyscrapers.SolvePuzzle(new[] { 0, 0, 0, 5, 0, 0, 3, 0, 6, 3, 4, 0, 0, 0, 3, 0, 0, 0, 2, 4, 0, 2, 6, 2, 2, 2, 0, 0 }).Select(x => string.Join(" ", x))));
            sw.Stop();
            ts2 = sw.Elapsed;
            Console.WriteLine($"3 总共花费{ts2.TotalMilliseconds}ms.");
            sw.Restart();
            Console.WriteLine(string.Join("\r\n", Skyscrapers.SolvePuzzle(new[] { 0, 0, 5, 0, 0, 0, 6, 4, 0, 0, 2, 0, 2, 0, 0, 5, 2, 0, 0, 0, 5, 0, 3, 0, 5, 0, 0, 3 }).Select(x => string.Join(" ", x))));
            sw.Stop();
            ts2 = sw.Elapsed;
            Console.WriteLine($"4 总共花费{ts2.TotalMilliseconds}ms.");
            sw.Restart();
            Console.WriteLine(string.Join("\r\n", Skyscrapers.SolvePuzzle(new[] { 0, 0, 5, 3, 0, 2, 0, 0, 0, 0, 4, 5, 0, 0, 0, 0, 0, 3, 2, 5, 4, 2, 2, 0, 0, 0, 0, 5 }).Select(x => string.Join(",", x))));
            sw.Stop();
            ts2 = sw.Elapsed;
            Console.WriteLine($"5 总共花费{ts2.TotalMilliseconds}ms.");
            sw.Restart();
            Console.WriteLine(string.Join("\r\n", Skyscrapers.SolvePuzzle(new[] { 0, 2, 3, 0, 2, 0, 0, 5, 0, 4, 5, 0, 4, 0, 0, 4, 2, 0, 0, 0, 6, 5, 2, 2, 2, 2, 4, 1 }).Select(x => string.Join(",", x))));
            sw.Stop();
            ts2 = sw.Elapsed;
            Console.WriteLine($"6 总共花费{ts2.TotalMilliseconds}ms.");
            sw.Restart();
            Console.WriteLine(string.Join("\r\n", Skyscrapers.SolvePuzzle(new[] { 0, 2, 3, 0, 2, 0, 0, 5, 0, 4, 5, 0, 4, 0, 0, 4, 2, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0 }).Select(x => string.Join(",", x))));
            sw.Stop();
            ts2 = sw.Elapsed;
            Console.WriteLine($"7 总共花费{ts2.TotalMilliseconds}ms.");
            sw.Restart();
            Console.WriteLine(string.Join("\r\n", Skyscrapers.SolvePuzzle(new[] { 0, 0, 0, 0, 5, 0, 4, 7, 0, 0, 0, 2, 2, 3, 0, 0, 3, 0, 0, 0, 0, 3, 0, 3, 0, 0, 5, 0 }).Select(x => string.Join(",", x))));
            sw.Stop();
            ts2 = sw.Elapsed;
            Console.WriteLine($"8 总共花费{ts2.TotalMilliseconds}ms.");
            sw.Restart();
            sw0.Stop();
            Console.WriteLine($"***** 总共花费{sw0.Elapsed.TotalMilliseconds}ms.");
        }
    }
}
