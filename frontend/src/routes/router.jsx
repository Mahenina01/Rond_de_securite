import { createBrowserRouter, Navigate } from "react-router";

import DashboardLayout from "../features/DashboardLayout";

import Dashboard from "../features/dashboard/Dashboard";
import Agents from "../features/agent/Agents";
import Devices from "../features/device/Devices";
import Reports from "../features/report/Reports";
import Settings from "../features/setting/Settings";
import SignIn from "../features/auth/SignIn";
import NotFound from "../features/notfound/NotFound";
import Locations from "../features/location/Locations";

const router = createBrowserRouter([
  {
    path: "/",
    element: <DashboardLayout />,
    children: [
      {
        index: true,
        element: <Navigate to="/dashboard" replace />,
      },
      {
        path: "dashboard",
        element: <Dashboard />,
      },
      {
        path: "agents",
        element: <Agents />,
      },
      {
        path: "devices",
        element: <Devices />,
      },
      { path: "locations", element: <Locations /> },
      {
        path: "reports",
        element: <Reports />,
      },
      {
        path: "settings",
        element: <Settings />,
      },
    ],
  },
  {
    path: "/signin",
    element: <SignIn />,
  },
  {
    path: "*",
    element: <NotFound />,
  },
  {
    path: "/forgot-password",
    element: <NotFound />,
  },
  {
    path: "/verify-email",
    element: <NotFound />,
  },
  {
    path: "/reinitialize-password",
    element: <NotFound />,
  },
]);

export default router;
