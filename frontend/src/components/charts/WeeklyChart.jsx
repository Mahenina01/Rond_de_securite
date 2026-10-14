import {
  BarChart,
  Bar,
  XAxis,
  YAxis,
  CartesianGrid,
  ResponsiveContainer,
  Tooltip,
} from "recharts";

const data = [
  { day: "Lun", value: 30 },
  { day: "Mar", value: 74 },
  { day: "Mer", value: 67 },
  { day: "Jeu", value: 95 },
  { day: "Ven", value: 57 },
  { day: "Sam", value: 84 },
  { day: "Aujourd'hui", value: 92 },
];

export default function WeeklyChart() {
  return (
    <div className="w-full h-64">
      <ResponsiveContainer width="100%" height="100%">
        <BarChart
          data={data}
          margin={{ top: 8, right: 4, left: -20, bottom: 0 }}
          barCategoryGap="35%"
        >
          <CartesianGrid vertical={false} stroke="#E5E7EB" strokeWidth={1} />

          <XAxis
            dataKey="day"
            axisLine={false}
            tickLine={false}
            tick={{
              fill: "#525252",
              fontSize: 12,
              fontWeight: 600,
            }}
            dy={8}
          />

          <YAxis
            domain={[0, 100]}
            ticks={[0, 20, 40, 60, 80, 100]}
            axisLine={false}
            tickLine={false}
            tick={{
              fill: "#737373",
              fontSize: 12,
            }}
          />

          <Tooltip
            cursor={{ fill: "#F3E8FF", opacity: 0.4 }}
            formatter={(value) => [`${value}`, "Valeur"]}
          />

          <Bar
            dataKey="value"
            fill="#A855E8"
            radius={[10, 10, 0, 0]}
            maxBarSize={36}
          />
        </BarChart>
      </ResponsiveContainer>
    </div>
  );
}
