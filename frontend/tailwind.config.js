/** @type {import('tailwindcss').Config} */
export default {
  content: ["./index.html", "./src/**/*.{js,ts,jsx,tsx}"],
  theme: {
    extend: {
      // Palette de couleurs
      colors: {
        primary: {
          DEFAULT: "#903BFF",
          hover: "#8C00FF",
          light: "#B47AFF",
          outline: "#8A38F5",
          waiting: "#A763E4",
        },

        secondary: {
          DEFAULT: "#64748B",
          hover: "#475569",
        },

        background: "#EEEEEE",
        surface: "#FFFFFF",

        text: {
          primary: "#000000",
          secondary: "#585858",
          muted: "#C3C3C3",
        },

        border: "#C3C3C3",

        success: {
          DEFAULT: "#16A34A",
          light: "#DCFCE7",
        },

        danger: {
          DEFAULT: "#DC2626",
          light: "#FEE2E2",
        },

        warning: {
          DEFAULT: "#D97706",
          light: "#FEF3C7",
        },
      },

      // Typographie
      fontFamily: {
        sans: ["Inter", "sans-serif"],
        heading: ["Inter", "sans-serif"],
      },

      fontSize: {
        "2xs": ["0.6875rem", { lineHeight: "1rem" }],
      },

      // Arrondis
      borderRadius: {
        card: "1rem",
        button: "0.625rem",
        input: "0.5rem",
      },

      // Ombres
      boxShadow: {
        card: "0 2px 8px rgb(15 23 42 / 5%)",
        dropdown: "0 8px 24px rgb(15 23 42 / 10%)",
      },

      // Espacements personnalisés
      spacing: {
        section: "1.5rem",
      },
    },
  },
  plugins: [],
};
